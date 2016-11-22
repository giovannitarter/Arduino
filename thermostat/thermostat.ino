#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <PubSubClient.h>

#include "configuration.h"
#include "wireless.h"
#include "ntp.h"
#include "pinio.h"
#include "dht12.h"


WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = MQTT_SERVER;

Ticker statusLedTck;
Ticker dataReaderTck;
Ticker timeoutTck;

uint8_t fireSend;

char temp_hostname[] = "esp8266_%s";
char temp_outTempTopic[] = "devices/%s/temperature";
char temp_outHumTopic[] = "devices/%s/humidity";
char temp_outSwitchTopic[] = "devices/%s/switch%d/state";
char temp_inSwitchTopic[] = "devices/%s/switch%d/cmd";


char espHostname[MAX_TOPIC];
char outTempTopic[MAX_TOPIC];
char outHumTopic[MAX_TOPIC];
char outSwitch0Topic[MAX_TOPIC];
char inSwitch0Topic[MAX_TOPIC];
char outSwitch1Topic[MAX_TOPIC];
char inSwitch1Topic[MAX_TOPIC];


long lastMsg = 0;
char msg[50];
int value = 0;

uint8_t myMac[6];
char macStr[13];

void setupPins() {
  pinOut(STATUS_LED);
  pinOut(RELAY0);
  pinOut(RELAY1);
}


void sendRelaysStatus() {

  if (getPinState(RELAY0)) {
      client.publish(outSwitch0Topic, "ON");  
  }
  else {
    client.publish(outSwitch0Topic, "OFF");
  }

  if (getPinState(RELAY1)) {
      client.publish(outSwitch1Topic, "ON");  
  }
  else {
    client.publish(outSwitch1Topic, "OFF");
  }
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
  if ((char)payload[0] == 'O' && (char)payload[1] == 'N') {
    setPin(RELAY0);
  
  } else {
      clearPin(RELAY0);  
  }
  sendRelaysStatus();

}


void callbackDataReader () {
  fireSend = 1;
}


void reconnect () {

  dataReaderTck.detach();

  clearPin(STATUS_LED);
  statusLedTck.attach_ms(100, togglePin, (uint8_t)STATUS_LED);

  if (getWifiStatus() != WL_CONNECTED) {
    setupWifi();
  }
  
  //setupNtp();
  
  if (! client.connected()) {
    client.connect(espHostname);
    sendRelaysStatus();
    client.subscribe(inSwitch0Topic);
    client.subscribe(inSwitch1Topic);
  }
  statusLedTck.detach();
  setPin(STATUS_LED);

  dataReaderTck.attach(SEND_DATA_PERIOD, callbackDataReader);
}


void sendHumTemp() {

  uint8_t temp, temp_dec, hum, hum_dec;
  getTempHum(&temp, &temp_dec, &hum, &hum_dec);

  char tmp[5];

  snprintf(tmp, 5, "%d.%d", temp, temp_dec);
  client.publish(outTempTopic, tmp);
  Serial.println(tmp);

  snprintf(tmp, 5, "%d.%d", hum, hum_dec);
  client.publish(outHumTopic, tmp);
  Serial.println(tmp);
}


void setup()
{
  int i;
  char * ptr;
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("BOOT");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.print("MAC: ");
  getMac(myMac);
  macToString(myMac, macStr);
  Serial.println(macStr);

  Serial.print("HOSTANAME: ");
  snprintf(espHostname, MAX_TOPIC, temp_hostname, macStr);
  Serial.println(espHostname);

  Serial.println("Topics:");
  snprintf(outTempTopic, MAX_TOPIC, temp_outTempTopic, macStr);
  Serial.println(outTempTopic);
  
  snprintf(outHumTopic, MAX_TOPIC, temp_outHumTopic, macStr);
  Serial.println(outHumTopic);
  
  snprintf(inSwitch0Topic, MAX_TOPIC, temp_inSwitchTopic, macStr, 0);
  Serial.println(inSwitch0Topic);
  
  snprintf(inSwitch1Topic, MAX_TOPIC, temp_inSwitchTopic, macStr, 1);
  Serial.println(inSwitch1Topic);
  
  snprintf(outSwitch0Topic, MAX_TOPIC, temp_outSwitchTopic, macStr, 0);
  Serial.println(outSwitch0Topic);
  
  snprintf(outSwitch1Topic, MAX_TOPIC, temp_outSwitchTopic, macStr, 1);
  Serial.println(outSwitch1Topic);
  
  setupPins();
  reconnect();
}


void loop()
{

  if (getWifiStatus() != WL_CONNECTED || client.connected() == 0) {
    reconnect();
  }

  if (fireSend) {
    sendHumTemp();
    fireSend = 0;
  }

  client.loop();

}
