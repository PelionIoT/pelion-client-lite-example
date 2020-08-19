// ----------------------------------------------------------------------------
// Copyright 2020 ARM Ltd.
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

#include "fota/fota_base.h"

#ifdef MBED_CLOUD_CLIENT_FOTA_ENABLE

#include "fota/fota_platform.h"
#include "fota/fota_component.h"

#include <stdio.h>
#include <assert.h>

// Componenet name - to be updated
// This name will be shown on portal and can be used as filter.
// When creating an update manifest for this component, this name must be specified as component-name.
// Note: must by 9 charecters long - including NULL terminator
#define DUMMY_COMPONENT_NAME "BLEexmpl"

// Our dummy component does not provide an interface for reading out currently installed FW.
// In order to report the current FW to Pelion Device Management portal we need to specify the
// initial FW version installed at factory.
#define DUMMY_FACTORY_VERSION "0.0.1"

static int example_installer(fota_candidate_iterate_callback_info *info);


int fota_platform_init_hook(bool after_upgrade)
{
    fota_component_desc_info_t component_descriptor = { 
        .candidate_iterate_cb = example_installer,
        .need_reboot = true 
    };
    int result = fota_component_add(
        &component_descriptor, 
        DUMMY_COMPONENT_NAME, DUMMY_FACTORY_VERSION);
    assert(0 == result);
    return 0;
}

int fota_platform_start_update_hook(const char *comp_name)
{
    // place holder for handling platform specific setup.
    // E.g. make sure candidtate storage is ready.
    return 0;
}

int fota_platform_finish_update_hook(const char *comp_name)
{
    // place holder for handling platform specific update finish logic
    return 0;
}

int fota_platform_abort_update_hook(const char *comp_name)
{
    // place holder for handling platform specific update finish logic
    return 0;
}

static int example_installer(fota_candidate_iterate_callback_info *info)
{
    switch (info->status) {
        case FOTA_CANDIDATE_ITERATE_START:
            printf(DUMMY_COMPONENT_NAME " - Installation started\n");
            printf(DUMMY_COMPONENT_NAME " - Installing ");
            break;
        case FOTA_CANDIDATE_ITERATE_FRAGMENT:
            printf(".");
            break;
        case FOTA_CANDIDATE_ITERATE_FINISH:
            printf("\n" DUMMY_COMPONENT_NAME " - Installation finished\n");
            break;
        default:
            return FOTA_STATUS_INTERNAL_ERROR;
    }
    return 0;
}

#endif  // MBED_CLOUD_CLIENT_FOTA_ENABLE