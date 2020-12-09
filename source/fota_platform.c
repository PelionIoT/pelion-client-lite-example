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

#include "fota/fota_platform.h"   // requied for implementing custom FOTA hooks (init, update start/finish/abort)
#include "fota/fota_component.h"  // required for registering custom component
#include "fota/fota_app_ifs.h"    // required for implementing custom install callback for Linux like targets
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

#if !defined(TARGET_LIKE_LINUX)
static int example_installer(fota_candidate_iterate_callback_info *info);
#endif

int fota_platform_init_hook(bool after_upgrade)
{
    fota_component_desc_info_t component_descriptor = { 
#if !defined(TARGET_LIKE_LINUX)
        .candidate_iterate_cb = example_installer,
#endif        
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

#if !defined(TARGET_LIKE_LINUX)
static int example_installer(fota_candidate_iterate_callback_info *info)
{
    switch (info->status) {
        case FOTA_CANDIDATE_ITERATE_START:
            printf(DUMMY_COMPONENT_NAME " - Installation started  (example)\n");
            printf(DUMMY_COMPONENT_NAME " - Installing ");
            break;
        case FOTA_CANDIDATE_ITERATE_FRAGMENT:
            printf(".");
            break;
        case FOTA_CANDIDATE_ITERATE_FINISH:
            printf("\n" DUMMY_COMPONENT_NAME " - Installation finished  (example)\n");
            break;
        default:
            return FOTA_STATUS_INTERNAL_ERROR;
    }
    return 0;
}
#elif !defined(USE_ACTIVATION_SCRIPT)  // e.g. Yocto target have different update activation logic residing outside of the example
// Simplified Linux use case example.
// For MAIN component update the the binary file current process is running.
// Simulate component update by just printing its name.
// After the installation callback returns, FOTA will "reboot" by calling mbed_client_default_reboot().
// Default implementation will restart the current process. The behaviour can be changed by injecting 
// MBED_CLOUD_CLIENT_CUSTOM_REBOOT define to the build and providing an alternative implementation for 
// mbed_client_default_reboot() function.


int fota_app_on_install_candidate(const char *candidate_fs_name, const manifest_firmware_info_t *firmware_info)
{
    int ret = FOTA_STATUS_SUCCESS;
    if (0 == strncmp(FOTA_COMPONENT_MAIN_COMPONENT_NAME, firmware_info->component_name, FOTA_COMPONENT_MAX_NAME_SIZE)) {
        // installing MAIN component
        ret = fota_component_install_main(candidate_fs_name);
        if (FOTA_STATUS_SUCCESS == ret) {
            printf("Successfully installed MAIN component\n");
            // FOTA does support a case where installer method reboots the system.
        }
    } else {
        printf("%s component installed (example)\n", firmware_info->component_name);
    }
    return ret;
}
#endif // !defined(TARGET_LIKE_LINUX)

#endif  // MBED_CLOUD_CLIENT_FOTA_ENABLE
