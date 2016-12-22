#ifndef _CONFIG_H_THERMOSTAT_
#define _CONFIG_H_THERMOSTAT_

void setupMDNS();
void discoverMqttServer();
void sendDiscovery();

void resolveZeroConf(
    char * service, 
    char * proto, 
    char * fallbackAddr,
    uint16_t fallbackPort,
    char * addr,
    uint16_t * port
    );
#endif
