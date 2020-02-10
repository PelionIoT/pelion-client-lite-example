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

#ifndef SETUP_H
#define SETUP_H

#if defined(USE_PLATFORM_CODE_OVERRIDE) && USE_PLATFORM_CODE_OVERRIDE == 1

#include "setup_override.h"

#else

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define PLAT_MILLI_PER_SECOND 1000
#define PLAT_NANO_PER_MILLI 1000000L
#define PLAT_NANO_PER_SECOND 1000000000L
#define PLAT_MILLI_TO_NANO(x) (((x) % PLAT_MILLI_PER_SECOND) * PLAT_NANO_PER_MILLI)

// Interval to update resource value in ms
#define INCREMENT_INTERVAL 25000

#define NETWORK_INTERFACE_COUNT 4
#define ETHERNET_INTERFACE 0
#define WIFI_INTERFACE 1

// Platform agnostic network status events.
typedef enum {
    // nsapi_connection_status_t types
    NETWORK_EVENT_GLOBAL_UP           = 115,
    NETWORK_EVENT_LOCAL_UP            = 116,
    NETWORK_EVENT_DISCONNECTED        = 117,
    NETWORK_EVENT_CONNECTING          = 118,
    NETWORK_EVENT_ERROR_UNSUPPORTED   = 119,
    // additional events to distinguish the network in question
    NETWORK_EVENT_GLOBAL_CONNECTING   = 120,
    NETWORK_EVENT_GLOBAL_DISCONNECTED = 121,
    NETWORK_EVENT_LOCAL_CONNECTING    = 122,
    NETWORK_EVENT_LOCAL_DISCONNECTED  = 123
} network_event_t;

typedef void (*main_t)(void);

#ifdef __linux__
#define MBED_CONF_RTOS_PRESENT 1
#endif

#if defined(MBED_CONF_RTOS_PRESENT) && (MBED_CONF_RTOS_PRESENT == 1)
typedef uintptr_t plat_mutex_id;
typedef uintptr_t plat_semaphore_id;

extern int32_t plat_semaphore_delete(plat_semaphore_id* semaphoreID);
extern int32_t plat_semaphore_release(plat_semaphore_id semaphoreID);
extern int32_t plat_semaphore_create(uint32_t count, plat_semaphore_id* semaphoreID);
extern int32_t plat_semaphore_wait(plat_semaphore_id semaphoreID, uint32_t millisec, int32_t* countersAvailable);
extern int32_t plat_mutex_create(plat_mutex_id* mutexID);
extern int32_t plat_mutex_wait(plat_mutex_id mutexID);
extern int32_t plat_mutex_release(plat_mutex_id mutexID);
extern int32_t plat_mutex_delete(plat_mutex_id* mutexID);
#endif // MBED_CONF_RTOS_PRESENT

// Initialize platform
// This function initializes screen and any other non-network
// related platform specific initializations required.
//
// @returns
//   0 for success, anything else for error
extern int init_platform(void);

extern bool init_storage(void);

// Initialize network connection. Use -1 to not register a tasklet.
extern bool init_connection(int8_t tasklet_network_event_handler_id);

// Close network connection
extern bool close_connection();

// Returns network interface.
extern void *get_network_interface(int32_t interface_index);

void* get_network_interface_at(int32_t interface_index);

bool connect_interface(int32_t interface_index, int8_t tasklet_network_event_handler_id);

bool disconnect_network_interface(int32_t interface_index);

bool is_valid_interface_index(int32_t interface_index);

// Toggle led (if available)
extern void toggle_led(void);

// Check if button has been pressed (if available)
extern uint8_t button_resource_clicked(void);
extern uint8_t button_unregister_clicked(void);

// Wait
extern void do_wait(int timeout_ms);

extern bool runProgram(main_t mainFunc);

#ifdef __cplusplus
}
#endif

#endif // defined(USE_PLATFORM_CODE_OVERRIDE) && USE_PLATFORM_CODE_OVERRIDE == 1

#endif
