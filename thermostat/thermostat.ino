#include <ESP8266WiFi.h>
#include "wireless.h"
#include "ntp.h"
#include "relays.h"

#include "dht12.h"
dht12 DHT12;
#define DHT12PIN 14


#include <PubSubClient.h>
const char* mqtt_server = "baseone.fritz.box";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void getData(float * temp, float * hum) {
  int chk = DHT12.read(DHT12PIN);

  Serial.print("Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
    Serial.println("OK"); 
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    Serial.println("Checksum error"); 
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    Serial.println("Time out error"); 
    break;
    default: 
    Serial.println("Unknown error"); 
    break;
  }

  *temp = DHT12.temperature / 10.0;
  *hum = DHT12.humidity / 10.0;
  
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    relayOn(RELAY1);
  
  
  } else {
      relayOff(RELAY1);  
  }

}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("BOOT");
  setupWifi();
  setupNtp();
  setupRelay();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}


void loop()
{
  if (! client.connected()) {
    client.connect("ESP8266Client");
  }
  
  //unsigned long ctime = getTime();
  //printTime(ctime);
  
  float temp, hum;
  getData(&temp, &hum);

  char tmp[5];
  sprintf(tmp, "%f", temp);
  client.publish("devices/temperature", String(temp).c_str());
  sprintf(tmp, "%f", hum);
  client.publish("devices/humidity", String(hum).c_str());

  Serial.println(temp);
  Serial.println(hum);

  client.loop();
  delay(5000);
}
