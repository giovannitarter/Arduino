#ifndef _CONFIGURATION_THERMOSTAT_H_
#define _CONFIGURATION_THERMOSTAT_H_


#define WIFI_SSID "GNET"
#define WIFI_PASS ""

#define MQTT_SERVER "baseone.fritz.box"
#define NTP_SERVER "ntp.inrim.it"

#define OTA_ADDRESS MQTT_SERVER
#define OTA_PORT 8000
#define OTA_LOCATION "/thermostat.bin"

#define OTA_PROTO "tcp"
#define OTA_SERVICE "esp"

#ifndef FW_VERSION
#define FW_VERSION "-1"
#endif

//in msec
#define NTP_TIMEOUT 5000 

#define RELAY0 12
#define RELAY1 13
#define STATUS_LED 16

#define DHT12PIN 14

#define MAX_TOPIC 40

#define SEND_DATA_PERIOD 30


#endif
