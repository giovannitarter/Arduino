SKETCH = parallel_print_server.ino

UPLOAD_PORT = /dev/ttyUSB0

#BOARD = nodemcuv2
#BOARD = d1
#UPLOAD_SPEED = 921600
#VERBOSE = 1

#BOARD = d1
#UPLOAD_SPEED = 921600
#FLASH_MODE = dio
#FLASH_DEF = 4M3M

BOARD = generic
UPLOAD_SPEED = 115200
FLASH_MODE = qio
FLASH_DEF = 4M3M

FW_VERSION ?= "0"

#CPP_EXTRA = -DDEBUG_ESP_HTTP_UPDATE -DDEBUG_ESP_PORT=Serial
CPP_EXTRA += -DFW_VERSION=\"$(FW_VERSION)\"
#CPP_EXTRA += -DMDNS_DEBUG_TX
BUILD_DIR = $(HOME)/Arduino/thermostat/out

include makeEspArduino.mk

edit: 
	gvim *.ino *.cpp *.h Makefile

monitor:
	picocom -e b $(UPLOAD_PORT) -b 115200 --imap lfcrlf

uploadm:
	make upload
	make monitor

.PHONY=
