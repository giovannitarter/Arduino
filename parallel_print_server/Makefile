SKETCH = parallel_print_server.ino

UPLOAD_PORT = /dev/ttyUSB0
BOARD = generic
UPLOAD_SPEED = 115200
FLASH_MODE = qio
FLASH_DEF = 4M1M
#VERBOSE = 1

FW_VERSION ?= "0"

#CPP_EXTRA = -DDEBUG_ESP_HTTP_UPDATE -DDEBUG_ESP_PORT=Serial
CPP_EXTRA += -DFW_VERSION=\"$(FW_VERSION)\"
#CPP_EXTRA += -DMDNS_DEBUG_TX
BUILD_DIR = $(HOME)/Arduino/thermostat/out

include makeEspArduino.mk

edit: 
	gvim *.ino *.cpp *.h

monitor:
	picocom -e b $(UPLOAD_PORT) -b 115200

.PHONY=
