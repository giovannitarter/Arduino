SKETCH = thermostat.ino

UPLOAD_PORT = /dev/ttyUSB0
BOARD = generic
UPLOAD_SPEED = 115200
FLASH_MODE = qio
#VERBOSE = 1

FW_VERSION ?= "0"

#CPP_EXTRA = -DDEBUG_ESP_HTTP_UPDATE -DDEBUG_ESP_PORT=Serial
CPP_EXTRA += -DFW_VERSION=\"$(FW_VERSION)\"
#CPP_EXTRA += -DMDNS_DEBUG_TX
BUILD_DIR = $(HOME)/Arduino/thermostat/out

include makeEspArduino.mk

.DEFAULT_GOAL := deploy
VERSION_FILE=.version

edit: 
	gvim *.cpp *.h *.ino

monitor:
	picocom -e b $(UPLOAD_PORT) -b 115200

deploy:
	VERSION=$$(cat $(VERSION_FILE)); \
	NEXT_VERSION=$$(expr $$VERSION + 1); \
	FST_MD5=$$(md5sum $(MAIN_EXE)); \
	FW_VERSION=$$NEXT_VERSION make all; \
	SND_MD5=$$(md5sum $(MAIN_EXE)); \
	if [ "$$FST_MD5" != "$$SND_MD5" ]; \
	then \
		cp $(MAIN_EXE) httpserver/ ; \
		echo $$NEXT_VERSION > $(VERSION_FILE); \
		cp $(VERSION_FILE) httpserver/thermostat.bin.version ; \
		echo "NEEDS_DEPLOY, VERSION: $$NEXT_VERSION"; \
	else \
		echo "NOT RECOMPILED"; \
	fi;


.PHONY=monitor edit