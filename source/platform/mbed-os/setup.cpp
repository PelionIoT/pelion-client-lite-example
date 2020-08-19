// ----------------------------------------------------------------------------
// Copyright 2019-2020 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------


#if defined(TARGET_LIKE_MBED) && (!defined(USE_PLATFORM_CODE_OVERRIDE) || USE_PLATFORM_CODE_OVERRIDE == 0)

#include "mbed.h"
#include "setup.h"
#include "mbed_error.h"
#include "eventOS_event.h"
#include "CellularDevice.h"

#if TARGET_NRF52_DK
#include "drivers/SerialWireOutput.h"
#endif

#define MBED_CONF_APP_ESP8266_TX MBED_CONF_APP_WIFI_TX
#define MBED_CONF_APP_ESP8266_RX MBED_CONF_APP_WIFI_RX

#include "mbed_trace.h"

#include <assert.h>

#define TRACE_GROUP "exam"

#if defined(MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS) && \
 (MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS == 1) && \
 defined(MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION) && \
 (MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION == 1)
#define MCC_USE_MBED_EVENTS 1
#endif

/* Callback function which informs about status of connected NetworkInterface.
 * */
static void network_status_callback(nsapi_event_t status, intptr_t param);
static void send_network_event(network_event_t network_event);
static NetworkInterface* network_interface=NULL;
static int8_t app_event_handler_id = -1;
static bool interface_connected = false;
static bool blocking_interface = false;
static SocketAddress sa;

#if defined(MBED_CONF_RTOS_PRESENT) && (MBED_CONF_RTOS_PRESENT == 1)
//! Semaphore structure
typedef struct plat_semaphore{
    plat_semaphore_id              semaphoreID;
    osSemaphoreAttr_t              osSemaphore;
    mbed_rtos_storage_semaphore_t  osSemaphoreStorage;
}plat_semaphore_t;

//! Mutex structure
typedef struct plat_mutex{
    plat_mutex_id             mutexID;
    osMutexAttr_t             osMutex;
    mbed_rtos_storage_mutex_t osMutexStorage;
}plat_mutex_t;

#endif // MBED_CONF_RTOS_PRESENT

// Define led on/off
#ifdef TARGET_STM
#define LED_ON (true)
#else // #ifdef TARGET_STM
#define LED_ON (false)
#endif // #ifdef TARGET_STM

#define LED_OFF (!LED_ON)

#ifndef MBED_CONF_APP_LED_PINNAME
#define MBED_CONF_APP_LED_PINNAME NC
#endif
DigitalOut led(MBED_CONF_APP_LED_PINNAME, LED_OFF);

#ifndef MBED_CONF_APP_BUTTON_RESOURCE
#define MBED_CONF_APP_BUTTON_RESOURCE NC
#endif
InterruptIn button_resource(MBED_CONF_APP_BUTTON_RESOURCE);

#ifndef MBED_CONF_APP_BUTTON_UNREGISTER
#define MBED_CONF_APP_BUTTON_UNREGISTER NC
#endif
InterruptIn button_unregister(MBED_CONF_APP_BUTTON_UNREGISTER);

static bool button_resource_pressed = false;
static bool button_unregister_pressed = false;

static void button_resource_press(void);
static void button_unregister_press(void);

void button_resource_press(void)
{
    button_resource_pressed = true;
}

void button_unregister_press(void)
{
    button_unregister_pressed = true;
}

uint8_t button_resource_clicked(void)
{
    if (button_resource_pressed) {
        button_resource_pressed = false;
        return true;
    }
    return false;
}

uint8_t button_unregister_clicked(void)
{
    if (button_unregister_pressed) {
        button_unregister_pressed = false;
        return true;
    }
    return false;
}

#if TARGET_NRF52_DK
// On NRF52_DK+ESP8266 combination the standard output is done via SWO as the
// serial UART is reserved for WiFI connectivity. This function will retarget
// the stdout to SWO.
FileHandle *mbed::mbed_override_console(int fd) {
    static SerialWireOutput swo_serial;
    return &swo_serial;
}
#endif

#if NDEBUG
// With this override on the release profile we can get rid of 2,5KB of excessive error logging
// and the printf floating point stuff it brings in thanks to non-constant format strings.
extern "C" {
mbed_error_status_t mbed_error(mbed_error_status_t error_status, const char *error_msg, unsigned int error_value, const char *filename, int line_number)
{
    printf("mbed_error(0x%x)\n", error_status);

    while (true) {
    }

    // this will never happen
    return error_status;
}
}
#endif

int init_platform(void)
{
    if(MBED_CONF_APP_BUTTON_RESOURCE != NC) {
        button_resource.fall(&button_resource_press);
    }
    if(MBED_CONF_APP_BUTTON_UNREGISTER != NC) {
        button_unregister.fall(&button_unregister_press);
    }
    return 0;
}

bool init_storage()
{
    return true;
}

#define xstr(s) str(s)
#define str(s) #s

const char* network_type(NetworkInterface *iface)
{
    if (iface->ethInterface()) {
        return "Ethernet";
    } else if (iface->wifiInterface()) {
        return "Wi-Fi " xstr(MBED_CONF_NSAPI_DEFAULT_WIFI_SSID);
    } else if (iface->meshInterface()) {
        return "Mesh";
    } else if (iface->cellularInterface()) {
        return "Cellular";
    } else if (iface->emacInterface()) {
        return "Emac";
    } else {
        return "Unknown";
    }
}

bool init_connection(int8_t tasklet_network_event_handler_id)
{
// Perform number of retries if network init fails.
#ifndef PLATFORM_CONNECTION_RETRY_COUNT
#define PLATFORM_CONNECTION_RETRY_COUNT 5
#endif
#ifndef PLATFORM_CONNECTION_RETRY_TIMEOUT
#define PLATFORM_CONNECTION_RETRY_TIMEOUT 1000
#endif
    printf("platform_init_connection()\n");

    network_interface = NetworkInterface::get_default_instance();
    if (network_interface == NULL) {
        printf("ERROR: No NetworkInterface found!\n");
        return false;
    }

    printf("Connecting with interface: %s\n", network_type(network_interface));
    if (network_interface->cellularInterface()) {
        CellularDevice *dev = CellularDevice::get_default_instance();
        dev->soft_power_off(); // switch off modem
        dev->hard_power_off(); // power supply off to cold boot modem on next connect()
    }

    // Delete the callback first in case the API is being called multiple times to prevent creation of multiple callbacks.
    network_interface->remove_event_listener(mbed::callback(&network_status_callback));
    network_interface->add_event_listener(mbed::callback(&network_status_callback));

    interface_connected = false;

    // Pass network status through to the application if it registers an event handler tasklet
    app_event_handler_id = tasklet_network_event_handler_id;
#if MCC_USE_MBED_EVENTS || MBED_CONF_CLIENT_APP_READ_STDIN_IN_EVENTLOOP
    // Application is running in single thread.
    if (network_interface->set_blocking(false) != NSAPI_ERROR_OK) {
        printf("WARN: Could not set non-blocking interface\n");
        blocking_interface = true;
    }

    if (network_interface->connect() != NSAPI_ERROR_OK) {
        return false;
    }

    // Stay here until we get a connection
    if (!blocking_interface) {
        for (int i = 1; i <= PLATFORM_CONNECTION_RETRY_COUNT; i++) {
            EventQueue *queue = mbed::mbed_event_queue();
            queue->dispatch_forever();
            if (interface_connected) {
                network_interface->get_ip_address(&sa);
                printf("IP: %s\n", (sa.get_ip_address() ? sa.get_ip_address() : "None"));
                return true;
            } else {
                printf("Failed to connect! Retry %d/%d\n", i, PLATFORM_CONNECTION_RETRY_COUNT);
                (void) network_interface->disconnect();
                do_wait(PLATFORM_CONNECTION_RETRY_TIMEOUT * i);
            }
        }
    } else {
        network_interface->get_ip_address(&sa);
        printf("IP: %s\n", (sa.get_ip_address() ? sa.get_ip_address() : "None"));
        interface_connected = true;
        return true;
    }
#else
    blocking_interface = true;
    for (int i = 1; i <= PLATFORM_CONNECTION_RETRY_COUNT; i++) {
        nsapi_error_t err = network_interface->connect();
        if (err == NSAPI_ERROR_OK || err == NSAPI_ERROR_IS_CONNECTED) {
            network_interface->get_ip_address(&sa);
            printf("IP: %s\n", (sa.get_ip_address() ? sa.get_ip_address() : "None"));
            interface_connected = true;
            return true;
        }
        printf("Failed to connect! error=%d. Retry %d/%d\n", err, i, PLATFORM_CONNECTION_RETRY_COUNT);
        (void) network_interface->disconnect();
        do_wait(PLATFORM_CONNECTION_RETRY_TIMEOUT * i);
    }
#endif
    return false;
}

bool close_connection()
{
    if (network_interface) {
        nsapi_error_t err = network_interface->disconnect();
        if (err == NSAPI_ERROR_OK) {
            network_interface->remove_event_listener(mbed::callback(&network_status_callback));
            network_interface = NULL;
            interface_connected = false;
            return true;
        }
    }
    return false;
}

void* get_network_interface(int32_t interface_index)
{
    if (interface_connected) {
        return network_interface;
    }

    return NULL;
}

bool connect_interface(int32_t interface_index, int8_t tasklet_network_event_handler_id)
{
    // XXX: support only one interface, at least for now
#if 1
    (void) interface_index;
    return init_connection(tasklet_network_event_handler_id);
#else
#ifndef MBED_CONF_APP_MULTIPLE_NETWORK_INTERFACES
    return (-1);
#else

    nsapi_error_t error = NSAPI_ERROR_UNSUPPORTED;
    if (network_interfaces[interface_index] == NULL) {
        WiFiInterface *iface = NULL;
        switch (interface_index) {
            case ETHERNET_INTERFACE:
                network_interfaces[interface_index] = EthernetInterface::get_default_instance();
                if (network_interfaces[interface_index] == NULL) {
                    error = NSAPI_ERROR_UNSUPPORTED;
                } else {
                    printf("Connecting to ethernet...\n");
                    error = network_interfaces[interface_index]->connect();
                }
                break;

            case WIFI_INTERFACE:
#if defined( MBED_CONF_NSAPI_DEFAULT_WIFI_SSID) && defined(MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD)
                iface = WiFiInterface::get_default_instance();
                if (iface == NULL) {
                    error = NSAPI_ERROR_UNSUPPORTED;
                } else {
                    printf("Connecting to wifi...\n");
                    network_interfaces[interface_index] = (NetworkInterface*) iface;
                    error = iface->connect(MBED_CONF_NSAPI_DEFAULT_WIFI_SSID,
                                           MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD,
                                           NSAPI_SECURITY_WPA_WPA2);
                }
#endif
                break;

            default:
                break;

        }
    }

    if (error != NSAPI_ERROR_OK && error != NSAPI_ERROR_IS_CONNECTED) {
        printf("Failed to connect! error=%d\n", error);
        disconnect_network_interface(interface_index);
        return (-1);
    }

    printf("IP: %s\n", network_interfaces[interface_index]->get_ip_address());
    return 0;

#endif
#endif
}

bool disconnect_network_interface(int32_t interface_index)
{
    // XXX: support only one interface, at least for now
#if 1
    return close_connection();
#else
    if (!is_valid_interface_index(interface_index)) {
        return;
    }
    if (network_interfaces[interface_index]) {
        network_interfaces[interface_index]->disconnect();
        network_interfaces[interface_index] = NULL;
    }
#endif
}

bool is_valid_interface_index(int32_t interface_index)
{
#if 1
    return true;
#else
    if (interface_index < ETHERNET_INTERFACE || interface_index > WIFI_INTERFACE) {
        return false;
    }

    return true;
#endif
}

void network_status_callback(nsapi_event_t status, intptr_t param)
{
#ifdef MCC_USE_MBED_EVENTS
    EventQueue *queue = mbed::mbed_event_queue();
#endif

    if (status == NSAPI_EVENT_CONNECTION_STATUS_CHANGE) {
        switch(param) {
            case NSAPI_STATUS_GLOBAL_UP:
                printf("Network initialized, NSAPI_STATUS_GLOBAL_UP\n");
#ifdef MCC_USE_MBED_EVENTS
                if (!interface_connected && !blocking_interface) {
                    queue->break_dispatch();
                }
#endif
                interface_connected = true;
                send_network_event(NETWORK_EVENT_GLOBAL_UP);
                break;
            case NSAPI_STATUS_LOCAL_UP:
                printf("NSAPI_STATUS_LOCAL_UP\n");
                send_network_event(NETWORK_EVENT_LOCAL_UP);
                break;
            case NSAPI_STATUS_DISCONNECTED:
                printf("NSAPI_STATUS_DISCONNECTED\n");
                interface_connected = false;
                send_network_event(NETWORK_EVENT_DISCONNECTED);
                break;
            case NSAPI_STATUS_CONNECTING:
                printf("NSAPI_STATUS_CONNECTING\n");
                send_network_event(NETWORK_EVENT_CONNECTING);
                break;
            case NSAPI_STATUS_ERROR_UNSUPPORTED:
                printf("NSAPI_STATUS_ERROR_UNSUPPORTED\n");
                send_network_event(NETWORK_EVENT_ERROR_UNSUPPORTED);
                break;
        }
    }
}

static void send_network_event(network_event_t network_event)
{
    arm_event_t event = {0};

    if (app_event_handler_id < 0) {
        // no event handler registered
        return;
    }

    event.receiver = app_event_handler_id;
    event.priority = ARM_LIB_MED_PRIORITY_EVENT;
    event.event_type = network_event;

    int ret = eventOS_event_send(&event);
    assert(ret == 0);
}

void toggle_led(void)
{
    if (MBED_CONF_APP_LED_PINNAME != NC) {
        led = !led;
    }
    else {
        printf("Virtual LED toggled\n");
    }
}

#if defined(MBED_CONF_RTOS_PRESENT) && (MBED_CONF_RTOS_PRESENT == 1)
int32_t plat_semaphore_delete(plat_semaphore_id* semaphoreID) {
    int32_t status;
    osStatus_t platStatus;
    plat_semaphore_t* semaphore;

    if(NULL == (void*)semaphoreID || NULL == (void*)*semaphoreID)
    {
        return -1;
    }

    semaphore = (plat_semaphore_t*)*semaphoreID;
    platStatus = osSemaphoreDelete((osSemaphoreId_t)semaphore->semaphoreID);
    if (osOK == platStatus)
    {
        free(semaphore);
        *semaphoreID = 0;
        status = 0;
    }
    else
    {
        status = -1;
    }

    return status;
}

int32_t plat_semaphore_release(plat_semaphore_id semaphoreID) {
    int32_t status = 0;
    osStatus_t platStatus = osOK;
    plat_semaphore_t* semaphore;

    if(0 == semaphoreID)
    {
        return -1;
    }

    semaphore = (plat_semaphore_t*)semaphoreID;
    platStatus = osSemaphoreRelease((osSemaphoreId_t)semaphore->semaphoreID);
    if (osOK == platStatus)
    {
        status = 0;
    }
    else
    {
        status = -1;
    }

    return status;
}

int32_t plat_semaphore_create(uint32_t count, plat_semaphore_id* semaphoreID) {
    int32_t status = 0;
    plat_semaphore_t* semaphore;
    if(NULL == (void*)semaphoreID)
    {
        return -1;
    }

    semaphore = (plat_semaphore_t*)malloc(sizeof(plat_semaphore_t));
    if (NULL == semaphore)
    {
        status = -1;
    }

    if(0 == status)
    {
        semaphore->osSemaphore.cb_mem = &semaphore->osSemaphoreStorage;
        semaphore->osSemaphore.cb_size = sizeof(semaphore->osSemaphoreStorage);
        memset(&semaphore->osSemaphoreStorage, 0, sizeof(semaphore->osSemaphoreStorage));

        semaphore->semaphoreID = (uintptr_t)osSemaphoreNew(1024, count, &semaphore->osSemaphore);
        if (0 == semaphore->semaphoreID)
        {
            free(semaphore);
            semaphore = NULL;
            status = -1;
        }
        else
        {
            *semaphoreID = (plat_semaphore_id)semaphore;
        }
    }
    return status;
}

int32_t plat_semaphore_wait(plat_semaphore_id semaphoreID, uint32_t millisec, int32_t* countersAvailable) {
    int32_t status = 0;
    plat_semaphore_t* semaphore;
    osStatus_t platStatus;
    if(0 == semaphoreID)
    {
        return -1;
    }

    semaphore = (plat_semaphore_t*)semaphoreID;
    platStatus = osSemaphoreAcquire((osSemaphoreId_t)semaphore->semaphoreID, millisec);

    if (osErrorTimeout == platStatus)
    {
        status = -2;
    }
    else if (platStatus != osOK)
    {
        status = -3;
    }

    if (NULL != countersAvailable)
    {
        *countersAvailable = osSemaphoreGetCount((osSemaphoreId_t)semaphore->semaphoreID);
    }
    return status;
}

int32_t plat_mutex_create(plat_mutex_id* mutexID) {
    int32_t status = 0;
    plat_mutex_t* mutex = NULL;

    if(NULL == (void*)mutexID)
    {
        return -1;
    }

    mutex = (plat_mutex_t*)malloc(sizeof(plat_mutex_t));
    if (NULL == mutex)
    {
        status = -1;
    }

    if (0 == status)
    {
        mutex->osMutex.name = NULL;
        mutex->osMutex.attr_bits = osMutexRecursive | osMutexRobust;
        mutex->osMutex.cb_mem = &mutex->osMutexStorage;
        mutex->osMutex.cb_size = sizeof(mutex->osMutexStorage);
        memset(&mutex->osMutexStorage, 0, sizeof(mutex->osMutexStorage));

        mutex->mutexID = (uintptr_t)osMutexNew(&mutex->osMutex);
        if (0 == mutex->mutexID)
        {
            free(mutex);
            mutex = NULL;
            status = -1;
        }
        else
        {
            *mutexID = (plat_mutex_id)mutex;
        }
    }
    return status;
}

int32_t plat_mutex_wait(plat_mutex_id mutexID) {
    int32_t status = 0;
    osStatus_t platStatus = osOK;
    plat_mutex_t* mutex = NULL;

    if(0 == mutexID)
    {
        return -1;
    }

    mutex = (plat_mutex_t*)mutexID;
    platStatus = osMutexAcquire((osMutexId_t)mutex->mutexID, osWaitForever);
    if (osOK == platStatus)
    {
        status = 0;
    }
    else
    {
        status = -1;
    }

    return status;
}

int32_t plat_mutex_release(plat_mutex_id mutexID) {
    int32_t status = 0;
    osStatus_t platStatus = osOK;
    plat_mutex_t* mutex = NULL;

    if (0 == mutexID)
    {
        return -1;
    }

    mutex = (plat_mutex_t*)mutexID;
    platStatus = osMutexRelease((osMutexId_t)mutex->mutexID);
    if (osOK == platStatus)
    {
        status = 0;
    }
    else
    {
        status = -1;
    }

    return status;
}

int32_t plat_mutex_delete(plat_mutex_id* mutexID) {
    int32_t status = 0;
    osStatus_t platStatus = osOK;
    plat_mutex_t* mutex = NULL;

    if (NULL == mutexID || 0 == *mutexID)
    {
        return -1;
    }

    mutex = (plat_mutex_t*)*mutexID;
    platStatus = osMutexDelete((osMutexId_t)mutex->mutexID);
    if (osOK == platStatus)
    {
        free(mutex);
        *mutexID = 0;
        status = 0;
    }
    else
    {
        status = -1;
    }

    return status;
}
#endif // MBED_CONF_RTOS_PRESENT

void do_wait(int timeout_ms)
{
    ThisThread::sleep_for(timeout_ms);
}

#endif // defined(TARGET_LIKE_MBED) && (!defined(USE_PLATFORM_CODE_OVERRIDE) || USE_PLATFORM_CODE_OVERRIDE == 0)
