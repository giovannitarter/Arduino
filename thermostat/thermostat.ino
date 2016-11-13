#include <ESP8266WiFi.h>
#include "wireless.h"
#include "ntp.h"
#include "relays.h"

#include "dht12.h"
dht12 DHT12;
#define DHT12PIN 14


void getData() {
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

  Serial.print("Humidity (%): ");
  Serial.println((float)DHT12.humidity, 2);

  Serial.print("Temperature (°C): ");
  Serial.println((float)DHT12.temperature, 2);

  
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

}


void loop()
{
  unsigned long ctime = getTime();
  printTime(ctime);
  getData();
  delay(5000); 
  relayOn(RELAY1);
  delay(2000);
  relayOff(RELAY1);
}
