#ifndef _CONFIG_H_THERMOSTAT_
#define _CONFIG_H_THERMOSTAT_


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Ticker.h>

#include "PubSubClient.h"
#include "RunningAverage.h"

#include "persistent_config.h"
#include "thermostat.h"
#include "wireless.h"
#include "ntp.h"
#include "pinio.h"
#include "dht.h"
#include "otaupdates.h"
#include "utils.h"


#ifndef FW_VERSION
#define FW_VERSION "-1"
#endif


//#define DEBUG

#ifdef DEBUG
#  define DEBUG_SERIAL(x) Serial.println({x});
#else
#  define DEBUG_SERIAL(x) ;
#endif


#define MAX_PAYLOAD 4
#define SWITCH_ON "ON"
#define SWITCH_OFF "OFF"


#define MQTT_SERVICE "mqtt"
#define MQTT_PROTO "tcp"


#define NTP_SERVER "ntp.inrim.it"


#define OTA_SERVICE "espupdater"
#define OTA_PROTO "tcp"
#define OTA_LOCATION "/thermostat.bin"


#define RUNNING_AVERAGE_SAMPLES 50
#define DISCOVERY_PERIOD 150
#define READ_SENSOR_PERIOD 3
#define SEND_DATA_PERIOD 150


//in msec
#define NTP_TIMEOUT 10000 

#define BT2 2
#define RELAY0 12
#define RELAY1 13
#define STATUS_LED 16

#define DHTPIN 14

#define MAX_TOPIC 40

#define temp_hostname "%s"
#define temp_outTempTopic "devices/%s/temperature"
#define temp_outHumTopic "devices/%s/humidity"

#define temp_outSwitchTopic "devices/%s/switch%d/state"
#define temp_inSwitchTopic "devices/%s/switch%d/cmd"
#define temp_availSwitchTopic "devices/%s/switch%d/avail"

#define temp_cfg_topic "%s/espconfig"


void setupMDNS();
void discoverMqttServer();
void sendDiscovery();


void initTopics();
void setupPins();
void reconnect();
void sendRelaysStatus();
void sendHumTemp();

void readSensor();

void callbackDataReader();
void callbackReadSensor();
void callbackDiscovery();
void callbackGetTime();


void callback(
        char * topic, 
        unsigned char * payload, 
        unsigned int length);

void resolveZeroConf(
    char * service, char * proto, 
    char * addr, uint16_t * port
    );

#endif
