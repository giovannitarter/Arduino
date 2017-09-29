#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "PubSubClient.h"

#include <Ticker.h>

#include "configuration.h"
#include "thermostat.h"
#include "wireless.h"
#include "ntp.h"
#include "pinio.h"
#include "dht.h"
#include "otaupdates.h"
#include "utils.h"

#define MAX_PAYLOAD 4
#define SWITCH_ON "ON"
#define SWITCH_OFF "OFF"
 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include "FS.h"
#include "persistent_config.h"

thermoCfg tcfg;

WiFiClient espClient;
PubSubClient client(espClient);

Ticker statusLedTck;
Ticker dataReaderTck;
Ticker getTimeTck;
Ticker sendDiscoveryTck;

uint8_t fireSend;
uint8_t readTime;
uint8_t fireDiscovery;
uint8_t fireSendRelay;

//Topics
char espHostname[MAX_TOPIC];
char outTempTopic[MAX_TOPIC];
char outHumTopic[MAX_TOPIC];

char outSwitch0Topic[MAX_TOPIC];
char inSwitch0Topic[MAX_TOPIC];
char availSwitch0Topic[MAX_TOPIC];

char outSwitch1Topic[MAX_TOPIC];
char inSwitch1Topic[MAX_TOPIC];
char availSwitch1Topic[MAX_TOPIC];


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

#define CONT_SIZE 800
 
void handleRoot() {

    int i;
    char temp[CONT_SIZE];
    char cont[CONT_SIZE];
    memset(&temp, 0, CONT_SIZE);
    memset(&cont, 0, CONT_SIZE);

    File f = SPIFFS.open("/config.json", "r+");
    f.read((uint8_t *) & cont,CONT_SIZE);
    f.close();        

	snprintf (temp, CONT_SIZE,

"<html>\n\
  <head>\n\
    <title>ESP8THERMO CONFIG PAGE</title>\n\
  </head>\n\
  <body>\n\
 ESP8THERMO CONFIG PAGE\n\
 <form method=\"POST\" action=\"/\" enctype=\"text/plain\" >\n\
  <textarea id=\"config\" name=\"config\" rows=\"20\" cols=\"50\">\n%s\n</textarea><br/><br/>\n\
  <input type=\"submit\" class=\"button\" value=\"Save\">\n\
  <input type=\"reset\" class=\"button\" value=\"Cancel\">\n\
 </form>\n\
  </body>\n\
</html>"
	,
    cont
    );

	server.send (CONT_SIZE, "text/html", temp );
}


void handleRootPost() {
    
    String cont;
    
    Serial.println("POST");
    Serial.println(server.arg("config"));
    Serial.println("ARGS END");
   
    cont = server.arg("config");

    File f = SPIFFS.open("/config.json", "w");
    f.print(cont);
    f.close();      
    
    handleRoot();
}


void httpConfig() {

    SPIFFS.begin();
    
    IPAddress local_IP(192,168,4,2);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    
    Serial.println("HTTP CONFIG");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(tcfg.name);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    server.on ("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handleRootPost);  
    server.begin();
}


void factoryConfig (thermoCfg * tcfg) {

        Serial.println("Factory config");
        
        char newname[20];
        memset(&newname, 0, 20);

        snprintf(newname, 20, "THERMO_%s", macStr + 9);
        upper(newname, 20);
        
        Serial.println(newname);
        strcpy(tcfg->name, newname); 

        strcpy(tcfg->essid, WIFI_SSID);
        strcpy(tcfg->pass, WIFI_PASS);
        
        strcpy(tcfg->server, MQTT_SERVER);
        tcfg->port = MQTT_PORT;
        
        strcpy(tcfg->otaserver, OTA_SERVER);
        tcfg->otaport = OTA_PORT;
        
        tcfg->sensType = SENS_DHT22;
        strcpy(tcfg->note, "a");
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
 
    factoryConfig(&tcfg);
    setup_config(&tcfg); 
    dht.type = tcfg.sensType;
 
    setupPins();

    int j;
    uint8_t btnStatus = 0;

    Serial.println("Press btn2 now to enter config.");
    for (j=0; j<20; j++) {
        btnStatus = getPinState(BT2);
        if (getPinState(BT2) == 0) {
            httpCfgEn = 1;
            break; 
            }
        delay(200);
    } 
    Serial.println("Config window stop.");
    
       

    //httpCfgEn = 1;
    if (httpCfgEn) { 
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

    DEBUG_SERIAL("sendRelaysStatus start");

    client.publish(availSwitch0Topic, "OFF");  
    client.publish(availSwitch1Topic, "ON");  
    
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
    
    DEBUG_SERIAL("sendRelaysStatus end");
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
        tcfg.otaserver, 
        tcfg.otaport
        );
    checkOTA(tcfg.otaserver, tcfg.otaport);
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
        //setupMDNS();
        setupNtp();
    }
    else {
        Serial.println("Wifi already connected");
    }
  
    Serial.println("\n\rTrying to resolve OTA updates addr"); 
    resolveZeroConf(
        OTA_SERVICE, OTA_PROTO,
        OTA_SERVER, OTA_PORT,
        tcfg.otaserver, &tcfg.otaport
        );
  
    
    checkUpdates();
     
    //Serial.println("\n\rTrying to resolve MQTT addr"); 
    //resolveZeroConf(MQTT_SERVICE, MQTT_PROTO,
    //                MQTT_SERVER, MQTT_PORT,
    //                mqttAddr, &mqttPort
    //                );
   
    //client.setServer(mqttAddr, mqttPort);
    client.setServer(tcfg.server, tcfg.port);
    
    Serial.println("MQTT SERVER:");
    Serial.println(tcfg.server);
    Serial.println(tcfg.port);

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
    Serial.println("INIT END\n");
  //getTimeTck.attach(5, callbackGetTime);
}


void sendDiscovery() {
    DEBUG_SERIAL("sendDiscovery start");
    if (client.connected()) {
        client.publish(discoveryTopic, tcfg.name, false);
    }
    DEBUG_SERIAL("sendDiscovery end");
}


void sendHumTemp() {

    uint8_t temp, temp_dec, hum, hum_dec;
    char tmp[50], res;
  
    DEBUG_SERIAL("sendHumTemp start");
  
    res = dht.read_dht();
    
    DEBUG_SERIAL("read dht done");

    temp = dht.temp;
    temp_dec = dht.temp_dec;
    hum = dht.hum;
    hum_dec = dht.hum_dec;
    
    if (res == DHTLIB_OK) {
 
        if (
            temp < 3 
            || temp > 80 
            || temp_dec < 0
            || temp_dec > 100
            || hum < 1
            || hum > 100
            || hum_dec < 0
            || hum_dec > 99
            )
        {
          snprintf(tmp, 
              50, 
              "TH read wrong: %d.%d t %d.%d h",
              temp,
              temp_dec,
              hum,
              hum_dec
          );
          Serial.println(tmp);
          Serial.println("toggle sens type");
          
          dht.toggle_type();
          tcfg.sensType = dht.type;
          writeConfig(&tcfg);
        }
        else {
          snprintf(tmp, 50, "%d.%d", temp, temp_dec);
          client.publish(outTempTopic, tmp);
          Serial.println(tmp);
  
          snprintf(tmp, 50, "%d.%d", hum, hum_dec);
          client.publish(outHumTopic, tmp);
          Serial.println(tmp);  
        }
    }
    else {
        Serial.println("DHT READ not OK, not sending messages");
    }     
         
    sendRelaysStatus();
    DEBUG_SERIAL("sendHumTemp end");
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
 
    //resNr = MDNS.queryService(service, proto);
    resNr = 0;
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
        Serial.printf("Falling back to: %s:%d\n", addr, *port);
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
    
    snprintf(availSwitch0Topic, MAX_TOPIC, temp_availSwitchTopic, tcfg.name, 0);
    Serial.println(availSwitch0Topic);
    
    snprintf(availSwitch1Topic, MAX_TOPIC, temp_availSwitchTopic, tcfg.name, 1);
    Serial.println(availSwitch1Topic);

}


