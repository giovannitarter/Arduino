#include "wireless.h"


void setup_wifi(char ssid[], char pass[]) {

  disconnectWifi();

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (getWifiStatus() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void disconnectWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
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
    sprintf(ptr, "%02x", mac_int[i]);
    ptr += 2;
  }
  
  mac_str[12] = 0;
}
