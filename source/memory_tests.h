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

#ifndef __MEMORY_TESTS_H__
#define __MEMORY_TESTS_H__

#ifdef MBED_CONF_MBED_CLIENT_ENABLE_CPP_API
#include "mbed-client/m2minterface.h"
#include "mbed-cloud-client/MbedCloudClient.h"
#endif
#include "mbed.h"

#ifdef MBED_CONF_MBED_CLIENT_ENABLE_CPP_API
// A function for creating a batch of resources for memory consumption purposes.
void m2mobject_test_set(M2MObjectList& object_list, MbedCloudClient &cloud_client);

// Print into serial the m2m object sizes and heap allocation sizes.
void m2mobject_stats(MbedCloudClient &cloud_client);
#endif

extern "C" void heap_stats(void);

#endif // !__MEMORY_TESTS_H__
