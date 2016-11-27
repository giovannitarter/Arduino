SKETCH = $(HOME)/Arduino/thermostat/thermostat.ino

UPLOAD_PORT = /dev/ttyUSB0
BOARD = generic
UPLOAD_SPEED = 115200
FLASH_MODE = qio
VERBOSE = 1

FW_VERSION ?= "0"

CPP_EXTRA = -DDEBUG_ESP_HTTP_UPDATE -DDEBUG_ESP_PORT=Serial
CPP_EXTRA += -DFW_VERSION=\"$(FW_VERSION)\"
BUILD_DIR = $(HOME)/Arduino/thermostat/out

include $(HOME)/Arduino/makeEspArduino/makeEspArduino.mk


edit: 
	gvim *.cpp *.h *.ino

monitor:
	picocom -e b $(UPLOAD_PORT) -b 115200

deploy:
	@echo $(CPP_EXTRA)

.PHONY=monitor edit
