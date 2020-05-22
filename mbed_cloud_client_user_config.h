// ----------------------------------------------------------------------------
// Copyright 2016-2020 ARM Ltd.
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


#ifndef MBED_CLOUD_CLIENT_USER_CONFIG_H
#define MBED_CLOUD_CLIENT_USER_CONFIG_H

#define MBED_CLOUD_CLIENT_ENDPOINT_TYPE             "default"

#if !defined(MBED_CLOUD_CLIENT_LIFETIME)
#define MBED_CLOUD_CLIENT_LIFETIME                  86400       /* 24 hours */
#endif

#if !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) && !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP) && !defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE)
#define MBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP
#endif

#ifdef MBED_CONF_MBED_CLIENT_SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      MBED_CONF_MBED_CLIENT_SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE
#else
    #define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE      512
#endif

#define MBED_CLOUD_CLIENT_FOTA_ENABLE 1

// Update Client definitions
#if MBED_CONF_APP_DEVELOPER_MODE == 1
    #define MBED_CLOUD_DEV_UPDATE_ID
    #define MBED_CLOUD_DEV_UPDATE_CERT
#endif

// FOTA support for bootloader header version 2.
#define MBED_CLOUD_CLIENT_FOTA_FW_HEADER_VERSION 2

// FOTA support for manifest version 1.
#define FOTA_MANIFEST_SCHEMA_VERSION 1

// Enable if bootloader supports encryption.
#define MBED_CLOUD_CLIENT_FOTA_ENCRYPTION_SUPPORT 0

#define USER_OMA_OBJECT_FILE "user_oma_lwm2m_object_defs.h"

#define PROTOMAN_SECURITY_ENABLE_CERTIFICATE

#if defined (PROTOMAN_USE_SSL_SESSION_RESUME) && (PROTOMAN_USE_SSL_SESSION_RESUME == 0)
#undef PROTOMAN_USE_SSL_SESSION_RESUME
#else
#define PROTOMAN_USE_SSL_SESSION_RESUME
#endif

#endif /* MBED_CLOUD_CLIENT_USER_CONFIG_H */
