# Okay to Wake Clock 2

This repository is a continuation of [okay-to-wake](https://github.com/szczys/okay-to-wake-clock) refactored for platformio plus improved functionality.

An ESP8266-based light clock using WS2812 LEDs for:

* Red during sleep time (overnight)
* Blue when wake time is approaching
* Green when it's time to get up
* Off during the day

Project log: https://hackaday.io/project/171671-improved-okay-to-wake-clock

## Development

* Install VSCode and the Platformio extension for VSCode
* Clone this repot into `~/Documents/PlatformIO/Projects/`
* Click the home icon in the bottom bar of VSCode and choose *Open Project*

    **IMPORTANT:** Once this has been flashed to the NodeMCU (ESP8266) board the first time you can then use OTA updates. In the `platformio.ini` file, uncomment the `upload_*` lines and update the IP address for your board.

## Implementation

This uses OTA updates so that it can be install inside of an existing okay to wake clock (robust case acts as diffuser, power suppy, all out of sight)
while still being possible to update

At power-up this will connect to WiFi, download time, and wait 10 minutes for an OTA update before turing WiFi off until next power cycle

## Power Considerations

I've measured the following power usage:

* LEDs on (yellow) and WiFi active: 120 mA
* LEDs off but WiFi active: 74 mA
* LEDs and WiFi off: 18 mA

Much of its life this will sit idle so it would be nice to implement further power-saving while anticipating a scheduled event. This is a task saved for future development.

