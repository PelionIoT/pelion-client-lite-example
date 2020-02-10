#!/bin/bash

## ----------------------------------------------------------------------------
## Copyright 2018 ARM Ltd.
##
## SPDX-License-Identifier: Apache-2.0
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
## ----------------------------------------------------------------------------

python ../mbed-cloud-client/tools/oma_lwm2m_data_model_tool.py -s ../mbed-cloud-client/tools/LWM2M.xsd -i 3200.xml 3201.xml 5000.xml -c ../source/user_oma_lwm2m_object_defs.h -m ../mbed-cloud-client/tools/oma_lwm2m_object_user_template.h

