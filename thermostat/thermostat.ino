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
 
#include <ESP8266mDNS.h>


WiFiClient espClient;
PubSubClient client(espClient);

char otaAddr[MAX_ADDR];
uint16_t otaPort;
char mqttAddr[MAX_ADDR];
uint16_t mqttPort;

Ticker statusLedTck;
Ticker dataReaderTck;
Ticker getTimeTck;

uint8_t fireSend;
uint8_t readTime;

char temp_hostname[] = "%s";
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
    Serial.printf(
        "Checking updates at %s:%d\n", 
        otaAddr, 
        otaPort
        );
    checkOTA(otaAddr, otaPort);
}


void reconnect () {

    unsigned long ctime;
  
    dataReaderTck.detach();
  
    clearPin(STATUS_LED);
    statusLedTck.attach_ms(100, togglePin, (uint8_t)STATUS_LED);
    
    ctime = getTime();
	Serial.print("Reconnecting at: ");
    printTime(ctime); 
  
    if (getWifiStatus() != WL_CONNECTED) {
        Serial.println("Reconnecting Wifi");
    	
        setupWifi();
        setupMDNS();
        setupNtp();
    }
    else {
        Serial.println("Wifi already connected");
    }
    
     
    Serial.println("Trying to resolve MQTT addr"); 
    resolveZeroConf(MQTT_SERVICE, MQTT_PROTO,
                    MQTT_SERVER, MQTT_PORT,
                    mqttAddr, &mqttPort
                    );
   
    client.setServer(mqttAddr, mqttPort);

    if (client.connected() == 0) {
        
        Serial.println("Reconnecting MQTT");
        client.connect(espHostname);
        if (client.connected()) {
            sendRelaysStatus();
            client.subscribe(inSwitch0Topic);
            client.subscribe(inSwitch1Topic);
            dataReaderTck.attach(SEND_DATA_PERIOD, callbackDataReader);
        }
        else {
            Serial.println("MQTT connection failed");
            delay(5000);
        }
    }
    

    statusLedTck.detach();
    setPin(STATUS_LED);
  
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

    if (MDNS.begin(espHostname)) {
  	    Serial.println("mDNS responder started");
  	}
    else {
    	Serial.println("Error setting up MDNS responder!");
    }
	MDNS.addService("esp", "tcp", 1883);
}

#define MAX_ADDR 20

void resolveZeroConf(
    char * service, 
    char * proto, 
    char * fallbackAddr,
    uint16_t fallbackPort,
    char * addr,
    uint16_t * port
    )
{
    int resNr;
    String strAddr;
    IPAddress tmpIp;
 
    resNr = MDNS.queryService(service, proto);
    if (resNr > 0) {

        strAddr = MDNS.IP(0).toString();
        *port = MDNS.port(0);
        
        strAddr.toCharArray(addr, MAX_ADDR);

        Serial.println("ZeroConf Service resolved");
        Serial.println(addr);
        Serial.println(*port);
    }
    else {
        WiFi.hostByName(fallbackAddr, tmpIp);
        strAddr = tmpIp.toString();
        strAddr.toCharArray(addr, MAX_ADDR);
        *port = fallbackPort;
        
        Serial.println("ZeroConf Service not found.");
        Serial.println("Falling back to:");
        Serial.println(addr);
        Serial.println(*port);
    }
}


void setup()
{
  int i;
  char * ptr;
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("\r\r\nBOOT");

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
    
  Serial.println("Trying to resolve OTA updates addr"); 
  resolveZeroConf(
        OTA_SERVICE, OTA_PROTO,
        OTA_ADDRESS, OTA_PORT,
        otaAddr, &otaPort
        );
  
  checkUpdates();
  
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
