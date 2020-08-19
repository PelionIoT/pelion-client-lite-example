// ----------------------------------------------------------------------------
// Copyright 2019 ARM Ltd.
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

#include "lwm2m_interface.h"
#include "lwm2m_registry_handler.h"
#include "setup.h"
#include "pdmc_client_example.h"
#include "dmc_connect_api.h"
#include "ns_hal_init.h"

#ifdef MBED_HEAP_STATS_ENABLED
#include "memory_tests.h"
#endif

#if defined(MCC_EXAMPLE_ENABLE_BLINKY) && (MCC_EXAMPLE_ENABLE_BLINKY == 1)
#include "blinky.h"
static Blinky blinky;
#endif

#include <stdio.h>
#include <string.h>

#ifdef __linux
#include <unistd.h>
#endif

static bool                 registered;
static bool                 register_called;

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
#define WAKE_UP_LOOP_INTERVAL_MS 300000
#define WAKE_UP_LOOP_INIT_EVENT 0
#define WAKE_UP_LOOP_WAKE_EVENT 1

static bool                 paused;
int8_t wake_up_handler = -1;
#endif

static int8_t               pdmc_event_handler_id;
int16_t                     counter;
registry_path_t             execute_path;
registry_callback_token_t   execute_token;

static void register_pdmc_client();
static void pdmc_client_registered();
static void pdmc_client_unregistered();
static void pdmc_client_registration_updated();
static void pdmc_client_error_event(lwm2m_interface_error_t error_code);
static void value_updated_event();
static void pdmc_event_handler(arm_event_t *event);
static void send_application_response(void *);
static bool init_example_resources();

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
static bool request_next_wake_up();
static void handle_wake_up();
#endif

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
static void update_progress(uint32_t progress, uint32_t total);
#endif

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
static void sleep_callback_function(void* context)
{
    // context is lwm2m_interface pointer
    (void)context;
    printf("Client is going to sleep\n");
    paused = true;
    pdmc_connect_pause();
    close_connection();
}
#endif

static registry_status_t button_callback(registry_callback_type_t type,
                                        const registry_path_t *path,
                                        const registry_callback_token_t *token,
                                        const registry_object_value_t *value,
                                        const registry_notification_status_t notification_status,
                                        registry_t *registry);

static registry_status_t pattern_callback(registry_callback_type_t type,
                                        const registry_path_t *path,
                                        const registry_callback_token_t *token,
                                        const registry_object_value_t *value,
                                        const registry_notification_status_t notification_status,
                                        registry_t *registry);

static registry_status_t blink_callback(registry_callback_type_t type,
                                        const registry_path_t *path,
                                        const registry_callback_token_t *token,
                                        const registry_object_value_t *value,
                                        const registry_notification_status_t notification_status,
                                        registry_t *registry);

static void pdmc_client_registered(void)
{
    registered = true;
    printf("Client registered\n");

    pdmc_endpoint_info_s endpoint_info;
    if (pdmc_connect_endpoint_info(&endpoint_info)) {
        printf("Endpoint Name: %s\r\n", endpoint_info.endpoint_name);
        printf("Device ID: %s\r\n", endpoint_info.device_id);
    }

#if defined(MCC_EXAMPLE_ENABLE_BLINKY) && (MCC_EXAMPLE_ENABLE_BLINKY == 1)
    blinky.start_loop();
    blinky.request_next_loop_event();
    blinky.request_automatic_increment_event();
#endif

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
    request_next_wake_up();
#endif

#ifdef MBED_HEAP_STATS_ENABLED
    heap_stats();
#endif
}

static void pdmc_client_unregistered(void)
{
    registered = false;
    register_called = false;
    printf("Client unregistered\n");

#ifdef MBED_HEAP_STATS_ENABLED
    heap_stats();
#endif
}

static void pdmc_client_registration_updated(void)
{
    printf("Client registration updated\n");
}

static void value_updated_event(void)
{
    printf("Value updated\n");
}

static void pdmc_client_error_event(lwm2m_interface_error_t error_code)
{
#if !defined(DISABLE_ERROR_DESCRIPTION) || (DISABLE_ERROR_DESCRIPTION == 0)
    const char *error;
    switch (error_code) {
        case LWM2M_INTERFACE_ERROR_NONE:
            error = "ErrorNone";
            break;
        case LWM2M_INTERFACE_ERROR_ALREADY_EXISTS:
            error = "AlreadyExists";
            break;
        case LWM2M_INTERFACE_ERROR_BOOTSTRAP_FAILED:
            error = "BootstrapFailed";
            break;
        case LWM2M_INTERFACE_ERROR_INVALID_PARAMETERS:
            error = "InvalidParameters";
            break;
        case LWM2M_INTERFACE_ERROR_NOT_REGISTERED:
            error = "NotRegistered";
            break;
        case LWM2M_INTERFACE_ERROR_TIMEOUT:
            error = "Timeout";
            break;
        case LWM2M_INTERFACE_ERROR_NETWORK_ERROR:
            error = "NetworkError";
            break;
        case LWM2M_INTERFACE_ERROR_RESPONSE_PARSE_FAILED:
            error = "ResponseParseFailed";
            break;
        case LWM2M_INTERFACE_ERROR_UNKNOWN_ERROR:
            error = "UnknownError";
            break;
        case LWM2M_INTERFACE_ERROR_MEMORY_FAIL:
            error = "MemoryFail";
            break;
        case LWM2M_INTERFACE_ERROR_NOT_ALLOWED:
            error = "NotAllowed";
            break;
        case LWM2M_INTERFACE_ERROR_SECURE_CONNECTION_FAILED:
            error = "SecureConnectionFailed";
            break;
        case LWM2M_INTERFACE_ERROR_DNS_RESOLVING_FAILED:
            error = "DnsResolvingFailed";
            break;
        case LWM2M_INTERFACE_ERROR_UNREGISTRATION_FAILED:
            error = "UnregistrationFailed";
            break;
        default:
            error = "";
    }
    printf("Error occurred : %s\n", error);

#endif // DISABLE_ERROR_DESCRIPTION

    printf("Error code : %d\n", error_code);
}

static void pdmc_event_handler(arm_event_t *event)
{
    if (event->event_type == 0 && event->event_id == 0) {
        return;
    }

    if (event->event_id == LWM2M_INTERFACE_OBSERVER_EVENT_OBJECT_REGISTERED) {
        pdmc_client_registered();
    } else if (event->event_id == LWM2M_INTERFACE_OBSERVER_EVENT_OBJECT_UNREGISTERED) {
        pdmc_client_unregistered();
    } else if (event->event_id == LWM2M_INTERFACE_OBSERVER_EVENT_REGISTRATION_UPDATED) {
        pdmc_client_registration_updated();
    } else if (event->event_id == LWM2M_INTERFACE_OBSERVER_EVENT_VALUE_UPDATED) {
        value_updated_event();
    } else if (event->event_id == LWM2M_INTERFACE_OBSERVER_EVENT_ERROR) {
        pdmc_client_error_event((lwm2m_interface_error_t)event->event_type);
    } else if (event->event_id == LWM2M_INTERFACE_OBSERVER_EVENT_BOOTSTRAP_DONE) {
        printf("Client bootstrapped\n");
    } else if (event->event_id == M2M_CLIENT_EVENT_SETUP_COMPLETED) {
        register_pdmc_client();
    } else {
        printf("Unhandled event: %d\n", event->event_id);
    }
}

registry_status_t button_callback(registry_callback_type_t type,
                                        const registry_path_t *path,
                                        const registry_callback_token_t *token,
                                        const registry_object_value_t *value,
                                        const registry_notification_status_t notification_status,
                                        registry_t *registry)
{
    if (type == REGISTRY_CALLBACK_VALUE_UPDATED) {
        printf("Counter resource set to %.*s\n", (int)value->generic_value.data.opaque_data->size, value->generic_value.data.opaque_data->data);
    }
    switch(notification_status) {
        case NOTIFICATION_STATUS_BUILD_ERROR:
            printf("Notification callback: (%d/%d/%d) error when building CoAP message\n", path->object_id, path->object_instance_id, path->resource_id);
            break;
        case NOTIFICATION_STATUS_RESEND_QUEUE_FULL:
            printf("Notification callback: (%d/%d/%d) CoAP resend queue full\n", path->object_id, path->object_instance_id, path->resource_id);
            break;
        case NOTIFICATION_STATUS_SENT:
            printf("Notification callback: (%d/%d/%d) Notification sent to server\n", path->object_id, path->object_instance_id, path->resource_id);
            break;
        case NOTIFICATION_STATUS_DELIVERED:
            printf("Notification callback: (%d/%d/%d) Notification delivered\n", path->object_id, path->object_instance_id, path->resource_id);
            break;
        case NOTIFICATION_STATUS_SEND_FAILED:
            printf("Notification callback: (%d/%d/%d) Notification sending failed\n", path->object_id, path->object_instance_id, path->resource_id);
            break;
        case NOTIFICATION_STATUS_SUBSCRIBED:
            printf("Notification callback: (%d/%d/%d) subscribed\n", path->object_id, path->object_instance_id, path->resource_id);
            break;
        case NOTIFICATION_STATUS_UNSUBSCRIBED:
            printf("Notification callback: (%d/%d/%d) subscription removed\n", path->object_id, path->object_instance_id, path->resource_id);
            break;
        default:
            break;
    }
    return REGISTRY_STATUS_OK;
}

registry_status_t blink_callback(registry_callback_type_t type,
                                        const registry_path_t *path,
                                        const registry_callback_token_t *token,
                                        const registry_object_value_t *value,
                                        const registry_notification_status_t notification_status,
                                        registry_t *registry)
{
    if (type == REGISTRY_CALLBACK_EXECUTE) {
        printf("POST executed\n");

        registry_path_t pattern_path;
        const char *pattern_str;
        registry_set_path(&pattern_path, 3201, 0, 5853, 0, REGISTRY_PATH_RESOURCE);
        registry_get_value_string(&pdmc_connect_get_interface()->endpoint.registry, &pattern_path, &pattern_str);

        memcpy(execute_token.token, token->token, token->token_size);
        execute_token.token_size = token->token_size;
        execute_path = *path;
#if defined(MCC_EXAMPLE_ENABLE_BLINKY) && (MCC_EXAMPLE_ENABLE_BLINKY == 1)
        blinky.start(pattern_str, strlen(pattern_str), false);
#endif
        eventOS_timeout_ms(&send_application_response, 100, NULL);
    }

    return REGISTRY_STATUS_OK;
}

void send_application_response(void *)
{
    send_final_response(&execute_path,
                        &(pdmc_connect_get_interface()->endpoint),
                        execute_token.token,
                        execute_token.token_size,
                        COAP_MSG_CODE_RESPONSE_CHANGED,
                        false);
}

registry_status_t pattern_callback(registry_callback_type_t type,
                                   const registry_path_t *path,
                                   const registry_callback_token_t *token,
                                   const registry_object_value_t *value,
                                   const registry_notification_status_t notification_status,
                                   registry_t *registry)
{
    if (type == REGISTRY_CALLBACK_VALUE_UPDATED) {
        printf("PUT received, new value: %.*s\n", (int)value->generic_value.data.opaque_data->size, value->generic_value.data.opaque_data->data);
    }

    return REGISTRY_STATUS_OK;
}

static registry_status_t unregister_callback(registry_callback_type_t type,
                                             const registry_path_t *path,
                                             const registry_callback_token_t *token,
                                             const registry_object_value_t *value,
                                             const registry_notification_status_t notification_status,
                                             registry_t *registry)
{
    printf("Unregister resource executed\n");
    if (notification_status == NOTIFICATION_STATUS_IGNORE) {
        // This status means we've just received the POST request.
        // We need to send response here and wait for it to be delivered before actually unregistering, to avoid
        // leaving server 'hanging', waiting for the response.
        send_final_response(path, &(pdmc_connect_get_interface()->endpoint), token->token, token->token_size, COAP_MSG_CODE_RESPONSE_CHANGED, true);
    } else if (notification_status == NOTIFICATION_STATUS_DELIVERED || notification_status == NOTIFICATION_STATUS_SEND_FAILED) {
        // Execute response has gone through and has been acknowledged, or sending the response has failed.
        pdmc_connect_close();
    }

    return REGISTRY_STATUS_OK;
}

void register_pdmc_client()
{
    pdmc_connect_register(get_network_interface(-1));
}

void init_pdmc_client(void)
{
    ns_hal_init(NULL, MBED_CLIENT_EVENT_LOOP_SIZE, NULL, NULL);
    pdmc_event_handler_id = eventOS_event_handler_create(pdmc_event_handler, 0);

    pdmc_connect_init(pdmc_event_handler_id);

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
        lwm2m_interface_set_queue_sleep_handler(pdmc_connect_get_interface(), sleep_callback_function);
#endif

    if (!init_example_resources()) {
        printf("Failed to create resources - exit!\n");
        return;
    }

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
    pdmc_connect_update_set_progress_handler(update_progress);
#endif

    register_called = true;
}

void pdmc_client_close()
{
    pdmc_connect_close();
}

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
void pdmc_client_resume()
{
    init_connection(-1);
    pdmc_connect_resume(get_network_interface(-1));
    paused = false;
    while(!registered) {
        do_wait(1000);
    }
}

bool is_pdmc_client_paused()
{
    return paused;
}
#endif

bool is_pdmc_client_register_called()
{
    return register_called;
}

bool init_example_resources()
{
    registry_status_t ret;
    registry_path_t path;
    registry_t *registry = &pdmc_connect_get_interface()->endpoint.registry;

    // Create resource for button count. Path of this resource will be: 3200/0/5501. GET
    if (pdmc_connect_add_cloud_resource(registry, &path, 3200, 0, 5501, false, button_callback)) {
        counter = 0;
        ret = registry_set_value_int(registry, &path, counter);
        if (ret != REGISTRY_STATUS_OK) {
            printf("registry_set_value_int() failed: (%d)\n", ret);
            return false;
        }
    } else {
        printf("Failed to create 3200/0/5501 resource!\n");
        return false;
    }

    // Create resource for led blinking pattern. Path of this resource will be: 3201/0/5853. PUT
    if (pdmc_connect_add_cloud_resource(registry, &path, 3201, 0, 5853, false, pattern_callback)) {
        ret = registry_set_value_string(registry, &path, (char *)"500:500:500:500", false);
        if (ret != REGISTRY_STATUS_OK) {
            printf("registry_set_value_string() failed: (%d)\n", ret);
            return false;
        }
    } else {
        printf("Failed to create 3201/0/5853 resource!\n");
        return false;
    }

    // Create resource for starting the led blinking. Path of this resource will be: 3201/0/5850. POST
    if (!pdmc_connect_add_cloud_resource(registry, &path, 3201, 0, 5850, false, blink_callback)) {
        printf("Failed to create 3201/0/5850 resource!\n");
        return false;
    }

    // Create resource for unregistering the device. Path of this resource will be: 5000/0/1.
    if (!pdmc_connect_add_cloud_resource(registry, &path, 5000, 0, 1, false, unregister_callback)) {
        printf("Failed to create 5000/0/1 resource!\n");
        return false;
    }

    printf("Resources created\n");

#ifdef MBED_HEAP_STATS_ENABLED
    heap_stats();
#endif

    return true;
}

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
void update_progress(uint32_t progress, uint32_t total)
{
    uint8_t percent = (uint8_t)((uint64_t)progress * 100 / total);
    printf("Update progress = %" PRIu8 "%%\n", percent);
}
#endif // MBED_CLOUD_CLIENT_SUPPORT_UPDATE

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
static void wake_up_event_handler_wrapper(arm_event_s *event)
{
    if (event->event_type == WAKE_UP_LOOP_WAKE_EVENT) {
        handle_wake_up();
    }
}

static bool request_next_wake_up() {
    if (wake_up_handler < 0) {
        wake_up_handler = eventOS_event_handler_create(wake_up_event_handler_wrapper, WAKE_UP_LOOP_INIT_EVENT);
    }

    arm_event_t event = { 0 };

    event.event_type = WAKE_UP_LOOP_WAKE_EVENT;
    event.receiver = wake_up_handler;
    event.sender =  wake_up_handler;
    event.data_ptr = NULL;
    event.priority = ARM_LIB_LOW_PRIORITY_EVENT;

    const int32_t delay_ticks = eventOS_event_timer_ms_to_ticks(WAKE_UP_LOOP_INTERVAL_MS);

    if (eventOS_event_send_after(&event, delay_ticks) == NULL) {
        return false;
    } else {
        return true;
    }
}

static void handle_wake_up()
{
    request_next_wake_up();
    if (is_pdmc_client_paused()) {
        printf("Calling Pelion Client resume()\r\n");
        pdmc_client_resume();
    }
}
#endif
