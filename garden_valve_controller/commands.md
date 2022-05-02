arduino-cli compile -v -b esp8266:esp8266:nodemcuv2:baud=460800 . -u -p /dev/ttyUSB0 && picocom -b 115200 /dev/ttyUSB0

arduino-cli compile -v -b esp8266:esp8266:nodemcuv2:baud=460800 -u -p /dev/ttyUSB0 --build-property build.extra_flags="-DSTASSID=\"\" -DSTAPSK=\"\"" .  && picocom -b 115200 /dev/ttyUSB0
picocom -b 115200 /dev/ttyUSB0

arduino-cli compile -v -b esp8266:esp8266:nodemcuv2:baud=460800 -u -p /dev/ttyACM0 --build-property build.extra_flags="-DSTASSID=\"\" -DSTAPSK=\"\"" .  && picocom -b 115200 /dev/ttyACM0
picocom -b 115200 /dev/ttyACM0

gvim *.ino *.c *.cpp *.h commands.md

arduino-cli compile -v -b esp32:esp32:esp32wrover -u -p /dev/ttyUSB0 --build-property build.extra_flags="-DSTASSID=\"\" -DSTAPSK=\"" .


