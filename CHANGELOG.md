# Changelog for Pelion Device Management Client Lite Reference Example

### Release 1.4.0-lite (21.09.2021)

* Fixed bug where application assumed that event_id and event_type are both 0 when event handler is initialized. Now uses `PDMC_CONNECT_STARTUP_EVENT_TYPE -1 ` from client's LWM2M interface when creating own event handler. 
* [Mbed OS] Updated ISM43362 WiFi driver to #3813a4b with fixes for logging and UDP socket handling.
* Bootloader changes:
    1. Changed the bootloader library name from `tools.lib` to `prebuilt-bl.lib`.
    1. The new bootloader library contains a bootloader binary image and an `mbed_lib.json` file for each target in the `prebuilt-bl/TARGET_target_name/TARGET_BL_INTERNAL_FLASH` directory. The `mbed_lib.json` file defines common configurations for the bootloader and the application, including the bootloader flash bank size; the storage type, address and size; firmware-over-the-air (FOTA) storage configurations; `header_format` and `restricted_size`.
    1. Added the `target.bootloader_img` parameter to the `mbed_lib.json` file of the bootloader. This parameter defines the path to the bootloader image.
    1. Set the storage configuration to `null` in the `mbed_app.json` file to ignore the default storage configuration. As a result, the build system uses the configuration defined by the `mbed_lib.json` file of the bootloader.
* [Mbed OS] Updated to 6.12.0:
    1. minimal-printf is now enabled by default. It can be disabled by adding `"target.printf_lib": "std"` to the application configuration. For example, using `float` type LWM2M resources would require disabling minimal-printf.

### Release 1.3.0-lite (07.12.2020)

- Changed how the example application behaves when network connect fails. Now the application retries to connect after a timeout.

### Release 1.2.1-lite (24.08.2020)

Fixed handling of partially written (due to power-cut) flash pages while installing the FW candidate.

### Release 1.2.0-lite (17.08.2020)

Updated applications and bootloaders to support manifest version 3. This enables support for FOTA library features:
- Update download resume (currently not supported for differential updates) - resume download in case power or connectivity loss.
- Support firmware candidate encryption (useful for external storage) - disabled by default.
- Component update - allow an application to register custom components and installers.
- Semantic version - firmware version is reported based on the semantic version format.
- Firmware version is reported on the `/14/0/2` resource.
- Manifest tool v2.0.0 is required to work with this FOTA library.
- Update x.509 certificate is replaced with a public key in uncompressed point format (X9.62) - requires less memory.
- An application can defer the update.

### Release 1.1.1-lite (05.06.2020)

Increased the default lifetime of `UDP_QUEUE` mode devices to 600 seconds. The earlier, shorter lifetime was causing intermittent issues where the device was unable to re-register before the lifetime expired.

### Release 1.1.0-lite (20.05.2020)

* Added Pelion E2E test library v0.2.6.
* Updated to Mbed OS 5.15.3, compatible also with Mbed OS 6.0.0-alpha-3.
* Added support for NB-IoT Altair modem on `DISCO_L475VG_IOT01A` board.
* Removed `"ppp-cell-iface.apn-lookup"` and `"ppp-cell-iface.baud-rate"` from `mbed_app.json`, as they are obsolete (and removed in Mbed OS 6.0). Detailed in example [issue #3](https://github.com/ARMmbed/pelion-client-lite-example/issues/3).
* Workaround for GCC LTO issue [#12871](https://github.com/ARMmbed/mbed-os/issues/12781) in Mbed OS 6.0.0 Alpha 3 to `pico_lte_size.json`.
* Added new configuration `mbed_app_baremetal_tls.json` for K64F and ESP8266 WiFi to compile for `baremetal` optimized mbedTLS configuration. To use this, first go to `mbed-cloud-client/tools` folder and run `./setup_optimized_mbedtls.sh` and then compile using `mbed compile -m K64F -t GCC_ARM --app-config mbed_app_baremetal_tls.json --profile profiles/pico_lte_size.json --profile mbed-os/tools/profiles/extensions/minimal-printf.json`
* Added support for delta-tool and delta updates.

### Release 1.0.0-lite (12.02.2020)

Initial alpha release for public preview. Not suitable for production use.
