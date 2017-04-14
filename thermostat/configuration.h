#ifndef _CONFIGURATION_THERMOSTAT_H_
#define _CONFIGURATION_THERMOSTAT_H_

#include "wificreds.h"


#define MAX_ADDR 20

#define MQTT_SERVICE "mqtt"
#define MQTT_PROTO "tcp"
//#define MQTT_SERVER "baseone"
#define MQTT_SERVER "g3srv"
//#define MQTT_SERVER "192.168.1.3"
#define MQTT_PORT 1883
#define NTP_SERVER "ntp.inrim.it"

#define OTA_SERVICE "espupdater"
#define OTA_PROTO "tcp"
#define OTA_ADDRESS MQTT_SERVER
#define OTA_PORT 8000
#define OTA_LOCATION "/thermostat.bin"

#define DISCOVERY_PERIOD 60

#ifndef FW_VERSION
#define FW_VERSION "-1"
#endif

//in msec
#define NTP_TIMEOUT 5000 

#define RELAY0 12
#define RELAY1 13
#define STATUS_LED 16

#define DHTPIN 14

#define MAX_TOPIC 40

#define SEND_DATA_PERIOD 5


#endif
