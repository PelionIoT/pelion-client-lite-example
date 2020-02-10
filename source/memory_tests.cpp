// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
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


#ifdef TARGET_LIKE_MBED

#ifdef MBED_HEAP_STATS_ENABLED
// used by print_heap_stats only
#include "mbed_stats.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#ifdef MBED_CONF_MBED_CLIENT_ENABLE_CPP_API
#include "mbed-client/m2mbase.h"
#endif
#endif

// fixup the compilation on AMRCC for PRIu32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "memory_tests.h"

#ifdef MBED_CONF_MBED_CLIENT_ENABLE_CPP_API
#include "mbed-client/m2mblockmessage.h"
#include "mbed-client/m2mdevice.h"
#include "mbed-client/m2mfirmware.h"
#include "mbed-client/m2minterfacefactory.h"
#include "mbed-client/m2mobject.h"
#include "mbed-client/m2mserver.h"
#include "mbed-client/m2msecurity.h"
#endif

#include "mbed.h"
#include "mbed_stats.h"

#include <assert.h>

extern "C"
void heap_stats(void)
{
#ifdef MBED_HEAP_STATS_ENABLED
    mbed_stats_heap_t stats;
    mbed_stats_heap_get(&stats);
    printf("**** current_size: %" PRIu32 "\n", stats.current_size);
    printf("**** max_size    : %" PRIu32 "\n", stats.max_size);
#endif // MBED_HEAP_STATS_ENABLED
}

#ifdef MBED_CONF_MBED_CLIENT_ENABLE_CPP_API
void m2mobject_test_set(M2MObjectList& object_list, MbedCloudClient &cloud_client)
{
#ifdef MBED_HEAP_STATS_ENABLED
    printf("*************************************\n");

    mbed_stats_heap_t stats;
    mbed_stats_heap_get(&stats);

    uint32_t initial = stats.current_size;

    const int object_count = 1;
    const int object_id_range_start = 90;
    const int object_id_range_end = object_id_range_start + object_count;

    const int object_instance_count = 5;
    const int resource_count = 5;

    int total_object_count = 0;
    int total_object_instance_count = 0;
    int total_resource_count = 0;

    for (int object_id = object_id_range_start; object_id < object_id_range_end; object_id++) {

        M2MObject *obj = M2MInterfaceFactory::create_object(object_id, cloud_client.m2minterface_handle());

        for (int object_instance_id = 0; object_instance_id < object_instance_count; object_instance_id++) {

            M2MObjectInstance* obj_inst = obj->create_object_instance(object_instance_id);

            assert(obj_inst != NULL);
            total_object_instance_count++;
            for (int resource_id = 0; resource_id < resource_count; resource_id++) {

                M2MResource* resource = obj_inst->create_dynamic_resource(resource_id, "9",
                                                                        M2MResourceInstance::INTEGER, true);

                assert(resource != NULL);

                resource->set_operation(M2MBase::GET_ALLOWED);
                resource->set_value(7);

                total_resource_count++;
            }
        }

        object_list.push_back(obj);
        total_object_count++;
    }

    printf("objects       : %d\n", total_object_count);
    printf("obj instances : %d\n", total_object_instance_count);
    printf("resources     : %d\n", total_resource_count);

    mbed_stats_heap_get(&stats);
    printf("heap used     : %" PRIu32 "\n", stats.current_size - initial);

    printf("*************************************\n");
#endif // MBED_HEAP_STATS_ENABLED
}

// Note: the mbed-os needs to be compiled with MBED_HEAP_STATS_ENABLED to get
// functional heap stats, or the mbed_stats_heap_get() will return just zeroes.
void m2mobject_stats(MbedCloudClient &cloud_client)
{
#ifdef MBED_HEAP_STATS_ENABLED
    printf("\n*** M2M object sizes in bytes ***\n");
    printf("M2MBase: %" PRIu32 "\n", sizeof(M2MBase));
    printf("M2MObject: %" PRIu32 "\n", sizeof(M2MObject));
    printf("M2MObjectInstance: %" PRIu32 "\n", sizeof(M2MObjectInstance));
    printf("M2MResource: %" PRIu32 "\n", sizeof(M2MResource));
    printf("M2MResourceInstance: %" PRIu32 "\n", sizeof(M2MResourceInstance));
    printf("M2MDevice: %" PRIu32 "\n", sizeof(M2MDevice));
    printf("M2MFirmware: %" PRIu32 "\n", sizeof(M2MFirmware));
    printf("M2MServer: %" PRIu32 "\n", sizeof(M2MServer));
    printf("M2MSecurity: %" PRIu32 "\n", sizeof(M2MSecurity));
    printf("M2MBlockMessage: %" PRIu32 "\n", sizeof(M2MBlockMessage));
    printf("*************************************\n\n");

    mbed_stats_heap_t stats;
    mbed_stats_heap_get(&stats);

    printf("*** M2M heap stats in bytes***\n");
    uint32_t initial = stats.current_size;


    // Basic object creation
    initial = stats.current_size;
    uint32_t before_object = initial;
    M2MObject *obj = M2MInterfaceFactory::create_object(10000, cloud_client.m2minterface_handle());
    mbed_stats_heap_get(&stats);
    printf("M2MObject heap size: %" PRIu32 "\n", stats.current_size - initial);
    initial = stats.current_size;

    M2MObjectInstance* obj_inst = obj->create_object_instance();
    mbed_stats_heap_get(&stats);
    printf("M2MObjectInstance heap size: %" PRIu32 "\n", stats.current_size - initial);

    initial = stats.current_size;
    M2MResource* res = obj_inst->create_dynamic_resource(1, "1", M2MResourceInstance::STRING, false);
    assert(res);
    mbed_stats_heap_get(&stats);
    printf("M2MResource heap size: %" PRIu32 "\n", stats.current_size - initial);

    delete obj;
    mbed_stats_heap_get(&stats);
    if (before_object != stats.current_size) {
        printf("Resource leaked: %" PRIu32 "bytes\n", stats.current_size - before_object);
    }
    printf("*************************************\n\n");
#endif // MBED_HEAP_STATS_ENABLED
}
#endif // MBED_CONF_MBED_CLIENT_ENABLE_CPP_API
#endif // TARGET_LIKE_MBED
