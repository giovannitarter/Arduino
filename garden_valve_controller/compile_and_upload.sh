#!/bin/bash

arduino-cli compile -v -b esp8266:esp8266:nodemcuv2:baud=460800 . -u -p /dev/ttyUSB0 && picocom -b 115200 /dev/ttyUSB0
