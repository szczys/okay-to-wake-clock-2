# Okay to Wake Clock 2

This repository is a continuation of
[okay-to-wake](https://github.com/szczys/okay-to-wake-clock) refactored for
PlatformIO plus improved functionality.

An ESP8266-based light clock using WS2812 LEDs for:

* Red during sleep time (overnight)
* Blue when wake time is approaching
* Green when it's time to get up
* Off during the day

Project log: https://hackaday.io/project/171671-improved-okay-to-wake-clock

## Schedule

The schedule can be set using the defaults in `wake_schedule.h`, or specifying a
remote file using the `SCHEDULE_SERVER_PATH` define.

The schedule file syntax should match the following:

```txt
# Start with Monday
# Format: blue, green, off, red
# example: 0600|0615|0645|0700
0600|0615|0715|1900
0600|0615|0715|1900
0600|0615|0715|1900
0600|0615|0715|1900
0600|0615|0715|1900
0615|0630|0730|1900
0615|0630|0730|1900
```

When a new schedule file is found and validated, it will be compared to the
schedule values stored in EEPROM using CRC32. If the new schedule is different,
it will be stored in EEPROM and used both for the current runtime and loaded
during future reboots.

## Development

* Install VSCode and the PlatformIO extension for VSCode
* Clone this repo into `~/Documents/PlatformIO/Projects/`
* Click the home icon in the bottom bar of VSCode and choose *Open Project*

    **IMPORTANT:** Once this has been flashed to the NodeMCU (ESP8266) board
    the first time you can then use OTA updates. In the `platformio.ini` file,
    uncomment the `upload_*` lines and update the IP address for your board.

## Implementation

This uses OTA updates so that it can be install inside of an existing okay to
wake clock (robust case acts as diffuser, power supply, all out of sight) while
still being possible to update

At power-up this will connect to WiFi, download time, and wait 10 minutes for
an OTA update before turning WiFi off until next power cycle

In PlatformIO there is a `nodemcuv2-ota` env with and `upload` option that can
be used to perform the OTA update.

## Power Considerations

I've measured the following power usage:

* LEDs on (yellow) and WiFi active: 120 mA
* LEDs off but WiFi active: 74 mA
* LEDs and WiFi off: 18 mA

Much of its life this will sit idle so it would be nice to implement further
power-saving while anticipating a scheduled event. This is a task saved for
future development.
