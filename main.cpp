// ----------------------------------------------------------------------------
// Copyright 2016-2019 ARM Ltd.
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
#include <stdio.h>

#include "setup.h"
#include "pdmc_client_example.h"
#include "mbed_trace.h"

#ifdef TARGET_LIKE_MBED
#include "mbed.h"
#include "eventOS_scheduler.h"
#endif // TARGET_LIKE_MBED

#if MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_DISPATCH_FROM_APPLICATION
#include "nanostack-event-loop/eventOS_scheduler.h"
#endif

#ifdef MBED_HEAP_STATS_ENABLED
#include "memory_tests.h"
#endif

#ifdef MBED_CONF_MBED_CLIENT_ENABLE_CPP_API
extern void client_register();
#endif

int main(void)
{
#ifdef TARGET_LIKE_MBED
    printf("Client build at: " __DATE__ " " __TIME__ "\n");
#endif

    mbed_trace_init();

    // Add empty line first, looks like RAAS is missing some bytes from the first trace line.
    // This line is needed test case to pass.
    printf("\n" );
    printf("Application ready\n" );

    (void) init_platform();

    if (!init_connection(-1)) {
        printf("ERROR - init_connection() failed!\n");
        return false;
    }

    // Print some statistics of the object sizes and heap memory consumption
    // if the MBED_HEAP_STATS_ENABLED is defined.
#ifdef MBED_HEAP_STATS_ENABLED
    heap_stats();
#endif

#ifdef TARGET_LIKE_MBED
    printf("Starting Device Management Client Lite\n");
#endif

    init_pdmc_client();

#if defined(MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS) && \
 (MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS == 1) && \
 defined(MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION) && \
 (MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION == 1)
    printf("Starting mbed eventloop..\n");

    eventOS_scheduler_mutex_wait();

    EventQueue *queue = mbed::mbed_event_queue();
    queue->dispatch_forever();


#elif MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_DISPATCH_FROM_APPLICATION
    printf("Starting eventloop..\n");

    eventOS_scheduler_mutex_wait();
    // this will block for ever
    eventOS_scheduler_run();
#else

    // Check if client is registering or registered, if true sleep and repeat.
    while (is_pdmc_client_register_called()) {
        do_wait(100);
    }
#endif

#ifdef MBED_HEAP_STATS_ENABLED
    heap_stats();
#endif

    return 0;
}
