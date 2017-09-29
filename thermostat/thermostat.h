#ifndef _CONFIG_H_THERMOSTAT_
#define _CONFIG_H_THERMOSTAT_

#include "configuration.h"

//#define DEBUG

#ifdef DEBUG
#  define DEBUG_SERIAL(x) Serial.println({x});
#else
#  define DEBUG_SERIAL(x) ;
#endif


void setupMDNS();
void discoverMqttServer();
void sendDiscovery();


void initTopics();
void setupPins();
void reconnect();
void sendRelaysStatus();
void sendHumTemp();

void upper(char str[], size_t n);

void callback(
        char * topic, 
        unsigned char * payload, 
        unsigned int length);

void resolveZeroConf(
    char * service, 
    char * proto, 
    char * fallbackAddr,
    uint16_t fallbackPort,
    char * addr,
    uint16_t * port
    );


struct thermoCfg {
    
    char name[MAX_ADDR];
    
    char essid[MAX_ADDR];
    char pass[MAX_ADDR];

    char server[MAX_ADDR];
    uint16_t port;
    
    char otaserver[MAX_ADDR];
    uint16_t otaport;

    uint16_t sensType;
    
    char note[MAX_ADDR];
};


typedef struct thermoCfg thermoCfg; 


#endif
