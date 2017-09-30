#ifndef _CONFIG_H_WIRELESS_
#define _CONFIG_H_WIRELESS_

#include "thermostat.h"

void setup_wifi(char ssid[], char pass[]);
uint8_t getWifiStatus();

void getMac(uint8_t mac[6]);
void macToString(uint8_t mac_int[6], char mac_str[13]);
void disconnectWifi();

#endif
