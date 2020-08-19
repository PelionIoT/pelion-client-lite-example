# Changelog for Pelion Device Management Client Lite Reference Example

### Release 1.2.0-lite (19.08.2020)

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
