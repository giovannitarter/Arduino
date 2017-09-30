SKETCH = thermostat.ino

#LOCAL_MCU=$(shell echo $LOCAL_MCU)
#LOCAL_MCU=1

FW_VERSION ?= $(shell cat .version)

ifeq ($(LOCAL_MCU),1)
UPLOAD_PORT = /dev/ttyUSB0
BOARD = nodemcuv2
UPLOAD_SPEED = 460800
FLASH_MODE = dio
FLASH_DEF = 4M3M
CPP_EXTRA += -DFW_VERSION=\"$(FW_VERSION)\" -DSKIP_UPDATE="1"
else
UPLOAD_PORT = /dev/ttyUSB0
BOARD = generic
UPLOAD_SPEED = 115200
FLASH_MODE = qio
FLASH_DEF = 4M3M
CPP_EXTRA += -DFW_VERSION=\"$(FW_VERSION)\"
endif


#VERBOSE = 1


#CPP_EXTRA = -DDEBUG_ESP_HTTP_UPDATE -DDEBUG_ESP_PORT=Serial


#CPP_EXTRA += -DMDNS_DEBUG_TX
BUILD_DIR = $(HOME)/Arduino/thermostat/out

#LIBS += "$(HOME)/Arduino/libraries/ArduinoJson"

include makeEspArduino.mk

LIBS += "$(HOME)/Arduino/libraries/ArduinoJson/third-party/"

.DEFAULT_GOAL := deploy
VERSION_FILE=.version

edit: 
	gvim *.ino *.cpp *.h

monitor:
	picocom -e b $(UPLOAD_PORT) -b 115200 --imap lfcrlf

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


uploadm:
	make upload
	make monitor


forcedeploy:
	make clean
	make deploy

.PHONY=
