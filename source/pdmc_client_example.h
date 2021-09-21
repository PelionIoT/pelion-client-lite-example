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
#ifndef PDMC_CLIENT_API_EXAMPLE_H
#define PDMC_CLIENT_API_EXAMPLE_H

void init_pdmc_client();

void pdmc_client_close();

// init connection with retry logic. Will reboot if connection
// does not succeed within reboot logic
void init_connection();

#ifdef MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE
void pdmc_client_resume();

bool is_pdmc_client_paused();
#endif

bool is_pdmc_client_register_called();

#endif //PDMC_CLIENT_API_EXAMPLE_H
