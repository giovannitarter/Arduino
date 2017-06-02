#ifndef _CONFIG_H_THERMOSTAT_
#define _CONFIG_H_THERMOSTAT_

void setupMDNS();
void discoverMqttServer();
void sendDiscovery();

void initConfig();
bool loadConfig();
void writeConfig();
void factoryConfig();


void initTopics();
void setupPins();
void reconnect();
void sendRelaysStatus();
void sendHumTemp();

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

    uint16_t sensType;
};


typedef struct thermoCfg thermoCfg; 


#endif
