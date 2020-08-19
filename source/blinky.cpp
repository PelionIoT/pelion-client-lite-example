// ----------------------------------------------------------------------------
// Copyright 2018 ARM Ltd.
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

#include "blinky.h"

#include "nanostack-event-loop/eventOS_event.h"
#include "nanostack-event-loop/eventOS_event_timer.h"
#include "pdmc_client_example.h"
#include "lwm2m_registry_handler.h"
#include "dmc_connect_api.h"
#include "setup.h"
#include "mbed-trace/mbed_trace.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TRACE_GROUP "blky"

#define BLINKY_TASKLET_LOOP_INIT_EVENT 0
#define BLINKY_TASKLET_PATTERN_INIT_EVENT 1
#define BLINKY_TASKLET_PATTERN_TIMER 2
#define BLINKY_TASKLET_LOOP_TIMER 3
#define BLINKY_TASKLET_AUTOMATIC_INCREMENT_TIMER 4

#define BUTTON_POLL_INTERVAL_MS 100

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
#define AUTOMATIC_INCREMENT_INTERVAL_MS 300000 // Update resource periodically every 300 seconds
#else
#define AUTOMATIC_INCREMENT_INTERVAL_MS 60000  // Update resource periodically every 60 seconds
#endif

int8_t Blinky::_tasklet = -1;
Blinky *static_blinky = NULL;

extern "C" {

static void blinky_event_handler_wrapper(arm_event_s *event)
{
    assert(event);
    if (event->event_type == BLINKY_TASKLET_LOOP_INIT_EVENT) {
        static_blinky->request_next_loop_event();
    } else {
        // the init event will not contain instance pointer
        Blinky *instance = (Blinky *)event->data_ptr;
        if(instance) {
            instance->event_handler(*event);
        }
    }
}

}

Blinky::Blinky()
: _pattern(NULL),
  _curr_pattern(NULL),
  _state(STATE_IDLE),
  _restart(false)
{
    _button_count = 0;
}

Blinky::~Blinky()
{
    // stop will free the pattern buffer if any is allocated
    stop();
    static_blinky = NULL;
}

void Blinky::create_tasklet()
{
    if (_tasklet < 0) {
        _tasklet = eventOS_event_handler_create(blinky_event_handler_wrapper, BLINKY_TASKLET_LOOP_INIT_EVENT);
        assert(_tasklet >= 0);
        static_blinky = this;
    }
}

void Blinky::start_loop()
{
    // create the tasklet, if not done already
    create_tasklet();
}

bool Blinky::start(const char* pattern, size_t length, bool pattern_restart)
{
    assert(pattern);

    // create the tasklet, if not done already
    create_tasklet();

    _restart = pattern_restart;

    // allow one to start multiple times before previous sequence has completed
    stop();

    _pattern = (char*)malloc(length+1);
    if (_pattern == NULL) {
        return false;
    }

    memcpy(_pattern, pattern, length);
    _pattern[length] = '\0';

    _curr_pattern = _pattern;

    return run_step();
}

void Blinky::stop()
{
    free(_pattern);
    _pattern = NULL;
    _curr_pattern = NULL;
    _state = STATE_IDLE;
}

int Blinky::get_next_int()
{
    int result = -1;

    char *endptr;

    int conv_result  = strtol(_curr_pattern, &endptr, 10);

    if (*_curr_pattern != '\0') {
        // ints are separated with ':', which we will skip on next time
        if (*endptr == ':') {
            endptr += 1;
            result = conv_result;
        } else if (*endptr == '\0') { // end of
            result = conv_result;
        } else {
            tr_debug("invalid char %c", *endptr);
        }
    }

    _curr_pattern = endptr;

    return result;
}

bool Blinky::run_step()
{
    int32_t delay = get_next_int();

    // tr_debug("patt: %s, curr: %s, delay: %d", _pattern, _curr_pattern, delay);

    if (delay < 0) {
        _state = STATE_IDLE;
        return false;
    }

    if (request_timed_event(BLINKY_TASKLET_PATTERN_TIMER, ARM_LIB_MED_PRIORITY_EVENT, delay) == false) {
        _state = STATE_IDLE;
        assert(false);
        return false;
    }

    toggle_led();

    _state = STATE_STARTED;

    return true;
}

void Blinky::event_handler(const arm_event_s &event)
{
    switch (event.event_type) {
        case BLINKY_TASKLET_PATTERN_TIMER:
            handle_pattern_event();
            break;
        case BLINKY_TASKLET_LOOP_TIMER:
            handle_buttons();
            break;
        case BLINKY_TASKLET_AUTOMATIC_INCREMENT_TIMER:
            handle_automatic_increment();
            break;
        case BLINKY_TASKLET_PATTERN_INIT_EVENT:
        default:
            break;
    }
}

void Blinky::handle_pattern_event()
{
    bool success = run_step();

    if ((!success) && (_restart)) {
        // tr_debug("Blinky restart pattern");
        _curr_pattern = _pattern;
        run_step();
    }
}

void Blinky::request_next_loop_event()
{
    request_timed_event(BLINKY_TASKLET_LOOP_TIMER, ARM_LIB_LOW_PRIORITY_EVENT, BUTTON_POLL_INTERVAL_MS);
}

void Blinky::request_automatic_increment_event()
{
    request_timed_event(BLINKY_TASKLET_AUTOMATIC_INCREMENT_TIMER, ARM_LIB_LOW_PRIORITY_EVENT, AUTOMATIC_INCREMENT_INTERVAL_MS);
}

// helper for requesting a event by given type after given delay (ms)
bool Blinky::request_timed_event(uint8_t event_type, arm_library_event_priority_e priority, int32_t delay)
{
    assert(_tasklet >= 0);

    arm_event_t event = { 0 };

    event.event_type = event_type;
    event.receiver = _tasklet;
    event.sender =  _tasklet;
    event.data_ptr = this;
    event.priority = priority;

    const int32_t delay_ticks = eventOS_event_timer_ms_to_ticks(delay);

    if (eventOS_event_send_after(&event, delay_ticks) == NULL) {
        return false;
    } else {
        return true;
    }
}

registry_status_t Blinky::increment_button_value()
{
    registry_t *registry = &pdmc_connect_get_interface()->endpoint.registry;
    registry_path_t path;
    registry_set_path(&path, 3200, 0, 5501, 0, REGISTRY_PATH_RESOURCE);
    int64_t current_value = -1;
    registry_get_value_int(registry, &path, &current_value);
    _button_count = ++current_value;
    return registry_set_value_int(registry, &path, _button_count);
}

void Blinky::handle_buttons()
{

    // this might be stopped now, but the loop should then be restarted after re-registration
    request_next_loop_event();

    if (is_pdmc_client_register_called()) {
        if (button_resource_clicked()) {
            if (increment_button_value() != REGISTRY_STATUS_OK) {
                printf("Failed to update button value!\n");
            } else {
                printf("Button resource manually updated. Value %d\n", _button_count);
            }
        }
        if (button_unregister_clicked()) {
#ifdef TARGET_LIKE_MBED
            printf("Unregister called.\n");
#endif
            pdmc_client_close();
        }
    }
}

void Blinky::handle_automatic_increment()
{
    // this might be stopped now, but the loop should then be restarted after re-registration
    request_automatic_increment_event();

    if (is_pdmc_client_register_called()) {
        if (increment_button_value() != REGISTRY_STATUS_OK) {
            printf("Failed to update button value!\n");
        } else {
            printf("Button resource automatically updated. Value %d\n", _button_count);
        }
    }
}
