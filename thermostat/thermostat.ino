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
#include "dht.h"
#include "otaupdates.h"

#define MAX_PAYLOAD 4
#define SWITCH_ON "ON"
#define SWITCH_OFF "OFF"
 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include "FS.h"
#include <ArduinoJson.h>

thermoCfg tcfg;

WiFiClient espClient;
PubSubClient client(espClient);

char otaAddr[MAX_ADDR];
uint16_t otaPort;
char mqttAddr[MAX_ADDR];
uint16_t mqttPort;

Ticker statusLedTck;
Ticker dataReaderTck;
Ticker getTimeTck;
Ticker sendDiscoveryTck;

uint8_t fireSend;
uint8_t readTime;
uint8_t fireDiscovery;
uint8_t fireSendRelay;

char espHostname[MAX_TOPIC];
char outTempTopic[MAX_TOPIC];
char outHumTopic[MAX_TOPIC];
char outSwitch0Topic[MAX_TOPIC];
char inSwitch0Topic[MAX_TOPIC];
char outSwitch1Topic[MAX_TOPIC];
char inSwitch1Topic[MAX_TOPIC];

char discoveryTopic[] = "espdiscovery";
char resetTopic[] = "espreset";

long lastMsg = 0;
char msg[50];
int value = 0;

uint8_t myMac[6];
char macStr[13];

DHT dht = DHT(DHTPIN, SENS_DHT22);
  
unsigned long my_ctime;

    
ESP8266WebServer server(80);
char httpCfgEn;
 
void handleRoot() {
	
    char temp[400];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	snprintf ( temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8THERMO CONFIG PAGE</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
  <textarea name=\"Text1\" cols=\"40\" rows=\"5\"></textarea>  \
  </body>\
</html>"
	);
	server.send ( 200, "text/html", temp );
}


void httpConfig() {

    IPAddress local_IP(192,168,4,22);
    IPAddress gateway(192,168,4,9);
    IPAddress subnet(255,255,255,0);
    
    Serial.println("HTTP CONFIG");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("AAAAAAAA");
    WiFi.softAPConfig(local_IP, gateway, subnet);

    server.on ("/", handleRoot); 
    server.begin();
}


void setup()
{
    int i;
    char * ptr;
   
    delay(2000);
 
    fireSend = 0;
    readTime = 0;
    fireDiscovery = 0;
    fireSendRelay = 0;
  
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Serial.println("\n\n\n\rBOOT");
  
    Serial.print("MAC: ");
    getMac(myMac);
    macToString(myMac, macStr);
    Serial.println(macStr);
    
    initConfig(); 
    dht.type = tcfg.sensType;
    
    setupPins();

    /*
    char btnStatus;
    while (1) {
        btnStatus = getPinState(BT2);
        if (btnStatus) {
            Serial.println("BT2 ON");
        }
        else {
            Serial.println("BT2 OFF");
        
        }
        delay(200); 
    }
    */

    if (getPinState(BT2) == 0) {
        httpCfgEn = 1; 
        httpConfig();
        return;
    };

    client.setCallback(callback);
    initTopics();    
    reconnect();
    
}


void loop()
{

  if (httpCfgEn) {
    server.handleClient();
    return;
  }

  client.loop();

  if (getWifiStatus() != WL_CONNECTED || client.connected() == 0) {
    Serial.println("\n\n\rDISCONNECTED!");
    reconnect();
  }
  
  if (fireDiscovery) {
    sendDiscovery();
    fireDiscovery = 0;
  }
  
  if (fireSendRelay) {
    sendRelaysStatus();
    fireSendRelay = 0;
  }

  if (fireSend) {
    sendHumTemp();
    fireSend = 0;
  }
  
  if (readTime) {
    my_ctime = getTime();
    printTime(my_ctime);
    readTime = 0;
  }

  client.loop();
}


void setupPins() {
    pinOut(STATUS_LED);
    pinOut(RELAY0);
    pinOut(RELAY1);
    pinIn(BT2);
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
  
  else if (strcmp(topic, resetTopic) == 0) {
        Serial.println("Reset received, reset");
        ESP.restart();
  }
  
  else {
    snprintf(err, 50, "wrong topic \"%s\"", topic);
    Serial.println(err);
  }
 
    fireSendRelay = 1;
}


void callbackDataReader () {
  fireSend = 1;
}


void callbackDiscovery () {
  fireDiscovery = 1;
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
    Serial.printf("\n\n\n\r\n");
} 


void reconnect () {

    int i;
    dataReaderTck.detach();
    sendDiscoveryTck.detach();
  
    clearPin(STATUS_LED);
    clearPin(RELAY0);
    clearPin(RELAY1);

    statusLedTck.attach_ms(100, togglePin, (uint8_t)STATUS_LED);
    
    my_ctime = getTime();
	Serial.print("Reconnecting at: ");
    printTime(my_ctime); 
  
    if (getWifiStatus() != WL_CONNECTED) {
        Serial.println("Reconnecting Wifi");
    	
        setupWifi();
        setupMDNS();
        setupNtp();
    }
    else {
        Serial.println("Wifi already connected");
    }
  
    Serial.println("\n\rTrying to resolve OTA updates addr"); 
    resolveZeroConf(
        OTA_SERVICE, OTA_PROTO,
        OTA_ADDRESS, OTA_PORT,
        otaAddr, &otaPort
        );
  
    
    checkUpdates();
     
    //Serial.println("\n\rTrying to resolve MQTT addr"); 
    //resolveZeroConf(MQTT_SERVICE, MQTT_PROTO,
    //                MQTT_SERVER, MQTT_PORT,
    //                mqttAddr, &mqttPort
    //                );
   
    //client.setServer(mqttAddr, mqttPort);
    client.setServer(tcfg.server, tcfg.port);
    
    Serial.println("MQTT SERVER:\n");
    Serial.println(tcfg.server);

    if (client.connected() == 0) {
        
        Serial.println("Reconnecting MQTT");
        client.connect(tcfg.name);
        if (client.connected()) {
            
            client.subscribe(inSwitch0Topic);
            client.subscribe(inSwitch1Topic);
            client.subscribe(resetTopic);
           
            for (i=0; i<5; i++) {
                delay(1000);
                client.loop();
            }
 
            dataReaderTck.attach(SEND_DATA_PERIOD, callbackDataReader);
            sendDiscoveryTck.attach(DISCOVERY_PERIOD, callbackDiscovery);
            
            fireDiscovery = 1;
            fireSend = 1; 
            fireSendRelay = 1; 
        }
        else {
            Serial.println("MQTT connection failed");
            delay(5000);
        }
    }
    

    statusLedTck.detach();
    setPin(STATUS_LED);
    Serial.println("INIT END\n\n\n");
  //getTimeTck.attach(5, callbackGetTime);
}


void sendDiscovery() {
    Serial.println("Sending discovery");
    if (client.connected()) {
        client.publish(discoveryTopic, tcfg.name, false);
    }
}


void sendHumTemp() {

  uint8_t temp, temp_dec, hum, hum_dec;
  
  dht.read_dht();
  temp = dht.temp;
  temp_dec = dht.temp_dec;
  hum = dht.hum;
  hum_dec = dht.hum_dec;

  char tmp[5];

  snprintf(tmp, 5, "%d.%d", temp, temp_dec);
  client.publish(outTempTopic, tmp);
  Serial.println(tmp);

  snprintf(tmp, 5, "%d.%d", hum, hum_dec);
  client.publish(outHumTopic, tmp);
  Serial.println(tmp);
            
  sendRelaysStatus();
}


void setupMDNS() {

    if (MDNS.begin(tcfg.name)) {
  	    Serial.println("mDNS responder started");
  	}
    else {
    	Serial.println("Error setting up MDNS responder!");
    }
	//MDNS.addService("esp", "tcp", 1883);
}


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


void factoryConfig () {

        char newname[20];
        snprintf(newname, 20, "THERMO_%s", macStr + 9);
        strcpy(tcfg.name, newname); 
        strcpy(tcfg.essid, WIFI_SSID);
        strcpy(tcfg.pass, WIFI_PASS);
        strcpy(tcfg.server, MQTT_SERVER);
        tcfg.port = MQTT_PORT;
        tcfg.sensType = SENS_DHT22;
}


void writeConfig() {
     
        File f;   
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& cfg = jsonBuffer.createObject();

        cfg["NAME"] = tcfg.name;
        cfg["ESSID"] = tcfg.essid;
        cfg["PASS"] = tcfg.pass;
        cfg["SRV"] = tcfg.server;
        cfg["SRVP"] = tcfg.port;
        cfg["SENS"] = tcfg.sensType;
        
        f = SPIFFS.open("/config.json", "w");
        Serial.println("config.json open");
        cfg.printTo(f);
        f.close();

}


bool loadConfig() {
    
    StaticJsonBuffer<200> jsonBuffer;
    File f = SPIFFS.open("/config.json", "r+");
    
    if (f) {
        Serial.println("config.json exist");
        JsonObject& myconfig = jsonBuffer.parseObject(f);
        myconfig.printTo(Serial);
        
        if (myconfig["NAME"]) {
            strcpy(tcfg.name, myconfig["NAME"]);
            if (strlen(tcfg.name) == 0) {
                return false;
            }
        }
        else {
            return false;
        }
        
        if (myconfig["SRV"]) {
            strcpy(tcfg.server, myconfig["SRV"]);
            if (strlen(tcfg.server) == 0) {
                return false;
            }
        }
        else {
            return false;
        }
        
        if (myconfig["ESSID"]) {
            strcpy(tcfg.essid, myconfig["ESSID"]);
            if (strlen(tcfg.essid) == 0) {
                return false;
            }
        }
        else {
            return false;
        }
        
        if (myconfig["PASS"]) {
            strcpy(tcfg.pass, myconfig["PASS"]);
            if (strlen(tcfg.pass) == 0) {
                return false;
            }
        }
        else {
            return false;
        }
        
        if (myconfig["SRVP"]) {
            tcfg.port = myconfig["SRVP"];
            if (tcfg.port == 0) { 
                return false;
            }
        }
        else {
            return false;
        }
        
        if (myconfig["SENS"]) {
            tcfg.sensType = myconfig["SENS"];
        }
        else {
            return false;
        }
    
        f.close();
    }
    
    else {
        return false;
    }

    return true;
}


void initConfig() {

    if (SPIFFS.begin()) {
        Serial.println("SPIFFS begin ok!");
        
        if (loadConfig() == false) {
            Serial.println("loadconfig fail");
            factoryConfig();
            writeConfig();
        }
    }
    else 
    {
        Serial.println("SPIFFS begin fail!");
    }
}


void initTopics() {
    
  
    Serial.print("HOSTANAME: ");
    snprintf(tcfg.name, MAX_TOPIC, temp_hostname, tcfg.name);
    Serial.println(tcfg.name);
  
    Serial.println("Topics:");
    snprintf(outTempTopic, MAX_TOPIC, temp_outTempTopic, tcfg.name);
    Serial.println(outTempTopic);
    
    snprintf(outHumTopic, MAX_TOPIC, temp_outHumTopic, tcfg.name);
    Serial.println(outHumTopic);
    
    snprintf(inSwitch0Topic, MAX_TOPIC, temp_inSwitchTopic, tcfg.name, 0);
    Serial.println(inSwitch0Topic);
    
    snprintf(inSwitch1Topic, MAX_TOPIC, temp_inSwitchTopic, tcfg.name, 1);
    Serial.println(inSwitch1Topic);
    
    snprintf(outSwitch0Topic, MAX_TOPIC, temp_outSwitchTopic, tcfg.name, 0);
    Serial.println(outSwitch0Topic);
    
    snprintf(outSwitch1Topic, MAX_TOPIC, temp_outSwitchTopic, tcfg.name, 1);
    Serial.println(outSwitch1Topic);

}


