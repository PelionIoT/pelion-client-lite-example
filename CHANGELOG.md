# Changelog for Pelion Device Management Client Lite Reference Example

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
