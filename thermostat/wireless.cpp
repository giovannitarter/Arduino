#include "wireless.h"
#include <ESP8266WiFi.h>
#include "configuration.h"


char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;


void printWifiInfo() {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setupWifi() {

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

}


void getMac(uint8_t mac[6]) {
  WiFi.macAddress(mac); 
}


uint8_t getWifiStatus() {
  return WiFi.status();
}


void macToString(uint8_t mac_int[6], char mac_str[13]) {
  
  char * ptr;
  uint8_t i;
  
  mac_str[0] = 0;
  ptr = mac_str;
  
  for(i=0; i<6; i++) {
    sprintf(ptr, "%x", mac_int[i]);
    ptr += 2;
  }
  
  mac_str[12] = 0;
}
