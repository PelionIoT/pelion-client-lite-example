/*
 * Copyright (c) 2019 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OMA_LWM2M_OBJECT_USER_TEMPLATE
#define OMA_LWM2M_OBJECT_USER_TEMPLATE

#include "oma_lwm2m_object_defs.h"
#include "lwm2m_registry_meta.h"
#include <inttypes.h>


const lwm2m_resource_meta_definition_t OMA_LWM2M_RESOURCE_DEFS_OBJECT_3200[] = {
    LWM2M_RESOURCE_DEFINITION(5501, LWM2M_RESOURCE_SINGLE_INSTANCE, LWM2M_RESOURCE_MANDATORY, LWM2M_RESOURCE_OPERATIONS_RW, LWM2M_RESOURCE_TYPE_INTEGER, "button_resource")
};


const lwm2m_resource_meta_definition_t OMA_LWM2M_RESOURCE_DEFS_OBJECT_3201[] = {
    LWM2M_RESOURCE_DEFINITION(5850, LWM2M_RESOURCE_SINGLE_INSTANCE, LWM2M_RESOURCE_MANDATORY, LWM2M_RESOURCE_OPERATIONS_E, LWM2M_RESOURCE_TYPE_STRING, "blink_resource"),
    LWM2M_RESOURCE_DEFINITION(5853, LWM2M_RESOURCE_SINGLE_INSTANCE, LWM2M_RESOURCE_MANDATORY, LWM2M_RESOURCE_OPERATIONS_RW, LWM2M_RESOURCE_TYPE_STRING, "pattern_resource")
};


const lwm2m_resource_meta_definition_t OMA_LWM2M_RESOURCE_DEFS_OBJECT_5000[] = {
    LWM2M_RESOURCE_DEFINITION(1, LWM2M_RESOURCE_SINGLE_INSTANCE, LWM2M_RESOURCE_MANDATORY, LWM2M_RESOURCE_OPERATIONS_E, LWM2M_RESOURCE_TYPE_INTEGER, "unregister")
};


#define USER_OMA_OBJECTS     LWM2M_OBJECT_DEFINITION(3200, LWM2M_OBJECT_SINGLE_INSTANCE, LWM2M_OBJECT_MANDATORY, "Button Object", 1, OMA_LWM2M_RESOURCE_DEFS_OBJECT_3200),\
    LWM2M_OBJECT_DEFINITION(3201, LWM2M_OBJECT_SINGLE_INSTANCE, LWM2M_OBJECT_MANDATORY, "Blinky Object", 2, OMA_LWM2M_RESOURCE_DEFS_OBJECT_3201),\
    LWM2M_OBJECT_DEFINITION(5000, LWM2M_OBJECT_SINGLE_INSTANCE, LWM2M_OBJECT_MANDATORY, "Unregister Object", 1, OMA_LWM2M_RESOURCE_DEFS_OBJECT_5000)

#endif  // OMA_LWM2M_OBJECT_USER_TEMPLATE
