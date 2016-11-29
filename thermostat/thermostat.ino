#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <PubSubClient.h>

#include <Ticker.h>

#include "configuration.h"
#include "thermostat.h"
#include "wireless.h"
#include "ntp.h"
#include "pinio.h"
#include "dht12.h"
#include "otaupdates.h"

#define MAX_PAYLOAD 4
#define SWITCH_ON "ON"
#define SWITCH_OFF "OFF"
 
#include "ESP8266mDNS.h"


WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = MQTT_SERVER;

Ticker statusLedTck;
Ticker dataReaderTck;
Ticker getTimeTck;

uint8_t fireSend;
uint8_t readTime;

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


void getMDNS(char * service, char * proto) {

   Serial.println("Sending mDNS query");
 
   int n = MDNS.queryService(service, proto); 
    
// Send out query for esp tcp services
    Serial.println("mDNS query done");
    if (n == 0) {
        Serial.println("no services found");
    }
    else {
        Serial.print(n);
        Serial.println(" service(s) found");
        for (int i = 0; i < n; ++i) {
        // Print details for each service found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(MDNS.hostname(i));
        Serial.print(" (");
        Serial.print(MDNS.IP(i));
        Serial.print(":");
        Serial.print(MDNS.port(i));
        Serial.println(")");
        }
    }
    Serial.println();
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


void callback(char * topic, unsigned char * payload, unsigned int length) {

  char err[50];
  char * payloadStr = (char *) payload;
  int maxPayload = MAX_PAYLOAD;
  
  if (length < maxPayload) {
    maxPayload = length;
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, inSwitch0Topic) == 0) {
    if (strncmp(payloadStr, SWITCH_ON, length) == 0) 
    {
      setPin(RELAY0);
    }
    else if (strncmp(
      payloadStr, 
      SWITCH_OFF, 
      maxPayload) == 0) {
      
      clearPin(RELAY0);
    }
    else {
      snprintf(err, 50, 
      "wrong message \"%s\" on topic \"%s\"",
      payload, 
      topic);
      Serial.println(err);
    }
  }
  
  else if (strcmp(topic, inSwitch1Topic) == 0) {
    if (strncmp(payloadStr, SWITCH_ON, maxPayload) == 0) {
      setPin(RELAY1);
    }
    else if (strncmp(payloadStr, SWITCH_OFF, maxPayload) == 0) {
      clearPin(RELAY1);
    }
    else {
      snprintf(err, 50, 
      "wrong message \"%s\" on topic \"%s\"",
      payload, 
      topic);
      Serial.println(err);
    }
  }
  
  else {
    snprintf(err, 50, "wrong topic \"%s\"", topic);
    Serial.println(err);
  }
  
  sendRelaysStatus();
}


void callbackDataReader () {
  fireSend = 1;
}

void callbackGetTime() {
  readTime = 1;	
}


void checkUpdates() {
    int resOta;
    char otaServer[20];
    int otaPort;
    IPAddress ip;
    String ipstring;
    int resNr;
 
    resOta = 0; 
    //check zeroconf updates 
    resNr = MDNS.queryService(OTA_SERVICE, OTA_PROTO);
    if (resNr > 0) {
        ip = MDNS.IP(0);
        ipstring = ip.toString();
        ipstring.toCharArray(otaServer, 20);
        Serial.printf("Checking updates at %s:%d\n", 
            otaServer, 
            MDNS.port(0)
        );
        resOta = checkOTA(otaServer, MDNS.port(0));
    }

    //check static updates 
    if (resOta == 0) {
        Serial.printf("Checking updates at %s:%d\n", 
            OTA_ADDRESS, 
            OTA_PORT
        );
        resOta = checkOTA(OTA_ADDRESS, OTA_PORT);
    }
}


void reconnect () {

    unsigned long ctime;
  
    dataReaderTck.detach();
  
    clearPin(STATUS_LED);
    statusLedTck.attach_ms(100, togglePin, (uint8_t)STATUS_LED);
  
    if (getWifiStatus() != WL_CONNECTED) {
    	setupWifi();
        setupMDNS();
        checkUpdates();
        setupNtp();
    }
    
    if (! client.connected()) {
      client.connect(espHostname);
      sendRelaysStatus();
      client.subscribe(inSwitch0Topic);
      client.subscribe(inSwitch1Topic);
    }
    statusLedTck.detach();
    setPin(STATUS_LED);
  
    dataReaderTck.attach(SEND_DATA_PERIOD, callbackDataReader);

    ctime = getTime();
	Serial.print("Reconnectiog at: ");
    printTime(ctime); 
  //getTimeTck.attach(5, callbackGetTime);
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


void setupMDNS() {
	if (!MDNS.begin("BB")) {
    	Serial.println("Error setting up MDNS responder!");
  	}
  		Serial.println("mDNS responder started");
	MDNS.addService("http", "tcp", 80); // Announce esp tcp service on port 8080
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
  fireSend = 1;
}


void loop()
{
  unsigned long ctime;

  if (getWifiStatus() != WL_CONNECTED || client.connected() == 0) {
    reconnect();
  }

  if (fireSend) {
    sendHumTemp();
    fireSend = 0;
  }

  if (readTime) {
    ctime = getTime();
    printTime(ctime);
    readTime = 0;
  }

  client.loop();
}
