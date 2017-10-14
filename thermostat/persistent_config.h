#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include "FS.h"
#include "aJSON.h"

#include "wificreds.h"
#include "thermostat.h"
#include "dht.h"

#define MAX_ADDR 20

#define CONFIG_FILE_PATH "/config.json"

#define DEFAULT_NAME "THERMO_XXXX"
#define DEFAULT_WIFI_SSID WIFI_SSID
#define DEFAULT_WIFI_PASS WIFI_PASS
#define DEFAULT_MQTT_SERVER "192.168.1.3"
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_OTA_SERVER DEFAULT_MQTT_SERVER
#define DEFAULT_OTA_PORT 8000
#define DEFAULT_SENS_TYPE SENS_DHT22
#define DEFAULT_NOTE ""


struct thermoCfg {
    
    char name[MAX_ADDR];
    
    char essid[MAX_ADDR];
    char pass[MAX_ADDR];

    char server[MAX_ADDR];
    uint16_t port;
    
    char otaserver[MAX_ADDR];
    uint16_t otaport;

    uint16_t sens_type;
    
    char note[MAX_ADDR];
};


typedef struct thermoCfg thermoCfg; 


void reset_config();
void factoryConfig(thermoCfg * tcfg, char name []);
void setup_config(thermoCfg *, char name []);
bool loadConfig(thermoCfg *);
void writeConfig(thermoCfg *);
void dump_cfg_file();
bool verify_config();
bool merge_config(char newcfg[]);

#endif
