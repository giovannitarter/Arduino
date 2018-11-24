#include "thermostat.h"
 
thermoCfg tcfg;

WiFiClient espClient;
PubSubClient client(espClient);

Ticker statusLedTck;
Ticker dataReaderTck;
Ticker getTimeTck;
Ticker sendDiscoveryTck;
Ticker readSensorTck;

RunningAverage ra_temp(RUNNING_AVERAGE_SAMPLES);
RunningAverage ra_hum(RUNNING_AVERAGE_SAMPLES);

uint8_t fireSensorRead;
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

char cfg_topic[MAX_TOPIC];


char discoveryTopic[] = "espdiscovery";
char resetTopic[] = "espreset";
char hass_status[] = "hass/status";

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


void setup()
{
    int i, j;
    char * ptr;
    char newname[20];
    uint8_t btnStatus = 0;
   
    delay(2000);
 
    fireSend = 0;
    readTime = 0;
    fireDiscovery = 0;
    fireSendRelay = 0;
  
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Serial.println("\n\n\n\rBOOT");
    
    Serial.println("VERSION: " FW_VERSION);

    Serial.print("MAC: ");
    getMac(myMac);
    macToString(myMac, macStr);
    Serial.println(macStr);

    memset(&newname, 0, 20);
    snprintf(newname, 20, "THERMO_%s", macStr + 9);
    upper(newname, 20);
    Serial.printf("Default name: %s\n", newname);
    
    setup_config(&tcfg, newname); 
    dht.type = tcfg.sens_type;
 
    setupPins();

    Serial.println("Press btn2 now to enter config.");
    btnStatus = 0;
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

    readSensor();

    client.setCallback(callback);
    initTopics();    
    
    disconnectWifi();
    reconnect();
}


void loop()
{

    if (httpCfgEn) {
      server.handleClient();
      return;
    }
    
    client.loop();
    yield();
    
    if (getWifiStatus() != WL_CONNECTED || client.connected() == 0) {
      Serial.println("\n\n\rDISCONNECTED!");
      reconnect();
    }
    
    client.loop();
    yield();
    
    if (fireSensorRead) {
      readSensor(); 
      fireSensorRead = 0;
    }
    
    client.loop();
    yield();
    
    if (fireDiscovery) {
      sendDiscovery();
      fireDiscovery = 0;
    }
    
    client.loop();
    yield();
    
    if (fireSendRelay) {
      sendRelaysStatus();
      fireSendRelay = 0;
    }

    client.loop();
    yield();
    
    if (fireSend) {
      sendHumTemp();
      fireSend = 0;
    }

    client.loop();
    yield();
    
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

    client.publish(availSwitch0Topic, "ON");  
    client.loop();
    client.publish(availSwitch1Topic, "ON");  
    client.loop();
    
    if (getPinState(RELAY0)) {
        client.publish(outSwitch0Topic, "ON");  
        client.loop();
    }
    else {
      client.publish(outSwitch0Topic, "OFF");
      client.loop();
    }

    if (getPinState(RELAY1)) {
        client.publish(outSwitch1Topic, "ON");  
        client.loop();
    }
    else {
      client.publish(outSwitch1Topic, "OFF");
      client.loop();
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
    
    Serial.print("> MQTT Message [");
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
            payloadStr, 
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
        payloadStr, 
        topic);
        Serial.println(err);
      }
    }
    
    else if (strcmp(topic, resetTopic) == 0) {

        uint16_t reset_in;
        unsigned long reset_time;
        
        reset_in = (myMac[4] << 8) + myMac[5];
        reset_in = reset_in % 30000;         
        Serial.printf("reset in %d millis\n", reset_in);  
        reset_time = millis() + reset_in;
            
        while(millis() < reset_time) {
            yield();
        }

        Serial.println("Reset!");
        ESP.restart();
    }
    
    else if (strcmp(topic, cfg_topic) == 0) {
      Serial.println("Cfg Topic");
      Serial.printf("%s\n", payloadStr);
      //reset_config();
      merge_config(payloadStr);
      //Serial.println("Reset");
      //ESP.restart();
    }
    else if (strcmp(topic, hass_status) == 0) {
        Serial.println("Hass Online");
        if (strstr(payloadStr, "online")) {
            fireDiscovery = 1;
        }
    }
    else {
      snprintf(err, 50, "wrong topic \"%s\"", topic);
      Serial.println(err);
    }
    
      fireSendRelay = 1;
}


void callbackDataReader() {
  fireSend = 1;
}


void callbackReadSensor() {
  fireSensorRead = 1;
}


void callbackDiscovery() {
  fireDiscovery = 1;
}


void callbackGetTime(){
  readTime = 1;	
}


void checkUpdates() {
    Serial.printf(
        "Checking updates at %s:%d\n", 
        tcfg.otaserver, 
        tcfg.otaport
        );
#ifndef SKIP_UPDATE
    checkOTA(tcfg.otaserver, tcfg.otaport);
#else
    Serial.println("Skipping update check because of builf cfg");
#endif
    Serial.println("");
} 


void reconnect () {

    int i;
    dataReaderTck.detach();
    sendDiscoveryTck.detach();
    readSensorTck.detach();

    clearPin(STATUS_LED);
    clearPin(RELAY0);
    clearPin(RELAY1);

    statusLedTck.attach_ms(100, togglePin, (uint8_t)STATUS_LED);
    
    my_ctime = getTime();
	Serial.print("Attempting connection at at: ");
    printTime(my_ctime); 
  
    if (getWifiStatus() != WL_CONNECTED) {
        Serial.println("Wifi down, reconnecting");
    	
        setup_wifi(tcfg.essid, tcfg.pass);
                
        //when we get here, getWifiStatus returns WL_CONNECTED
        setupNtp(); 

        //setupMDNS();
    }
    else {
        Serial.println("Wifi already connected");
    }
    
    if (getWifiStatus() == WL_CONNECTED) {
        my_ctime = getTime();
        printTime(my_ctime);
    }
  
    Serial.println("\n\rTrying to resolve OTA updates addr"); 
    resolveZeroConf(
        OTA_SERVICE, OTA_PROTO,
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
    
    Serial.printf("> MQTT SERVER: %s:%d\n", tcfg.server, tcfg.port);
    if (client.connected() == 0) {
        
        Serial.println("> MQTT not connected, attempting connection");
        client.connect(tcfg.name);
        
        if (client.connected()) {
            
            Serial.println("> MQTT connected");
            
            client.subscribe(inSwitch0Topic);
            client.subscribe(inSwitch1Topic);
            client.subscribe(resetTopic);
            client.subscribe(cfg_topic);
            client.subscribe(hass_status);
           
            Serial.println("> MQTT processing pending messages");
            for (i=0; i<5; i++) {
                
                //time for messages to be received
                delay(500);

                Serial.println("> MQTT .");
                client.loop();

            }
            Serial.println("> MQTT processing pending messages end");
 
            dataReaderTck.attach(SEND_DATA_PERIOD, callbackDataReader);
            sendDiscoveryTck.attach(DISCOVERY_PERIOD, callbackDiscovery);           
            readSensorTck.attach(READ_SENSOR_PERIOD, callbackReadSensor);           
 
            fireDiscovery = 1;
            fireSend = 1; 
            fireSendRelay = 1; 
        }
        else {
            Serial.println("> MQTT connection failed");
            delay(5000);
        }
    }
    else {
        Serial.println("> MQTT connected");
    }
    Serial.println("");
    

    statusLedTck.detach();
    setPin(STATUS_LED);
    Serial.println("INIT END\n");
  //getTimeTck.attach(5, callbackGetTime);
}


void sendDiscovery() {
    Serial.println("> MQTT Sending Discovery");
    DEBUG_SERIAL("sendDiscovery start");
    if (client.connected()) {
        client.publish(discoveryTopic, tcfg.name, false);
    }
    DEBUG_SERIAL("sendDiscovery end");
}


void readSensor() {
    
    //Serial.println("ReadSensor");
    float temp = 0.0, hum = 0.0;
    uint8_t res;    
    char tmp[50];

    res = dht.read_dht();
    DEBUG_SERIAL("read dht done");

    if (res == DHTLIB_OK) {
 
        temp = dht.temp + (dht.temp_dec / 10.0) ;
        ra_temp.addValue(temp);
        
        hum = dht.hum + (dht.hum_dec / 10.0) ;
        ra_hum.addValue(hum);

    }

    else if (res == DHTLIB_ERROR_VALUE) {
        snprintf(
            tmp, 
            50, 
            "TH read wrong: %d.%d t %d.%d h",
            dht.temp,
            dht.temp_dec,
            dht.hum,
            dht.hum_dec
          );
          Serial.println(tmp);
          Serial.println("toggle sens type");
          
          dht.toggle_type();
          tcfg.sens_type = dht.type;
          writeConfig(&tcfg);
      
    }
    
    else {
        Serial.printf("DHT READ not OK (res: %d), not sending messages\n", res);
    } 
}


void sendHumTemp() {
    
    DEBUG_SERIAL("sendHumTemp start");

    char tmp_temp[10], tmp_hum[10];
    float hum, temp;

    if (ra_temp.getCount() > 0 && ra_hum.getCount() > 0) {

        dtostrf(ra_temp.getAverage(), 3, 1, tmp_temp);
        client.publish(outTempTopic, tmp_temp);
  
        dtostrf(ra_hum.getAverage(), 3, 1, tmp_hum);
        client.publish(outHumTopic, tmp_hum);
        
        Serial.printf("> MQTT sending data %s %s\n", tmp_temp, tmp_hum);  
    }
    else {
        Serial.println("Not sending data, running averages empty");
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

        Serial.printf("ZeroConf Service resolved: %s:%d\n", addr, *port);
    }
    else {
        Serial.printf("ZeroConf Service not found. Resolving %s\n", addr);
        WiFi.hostByName(addr, tmpIp);
        strAddr = tmpIp.toString();
        strAddr.toCharArray(addr, MAX_ADDR);
        Serial.printf("Addr resolved: %s:%d\n", addr, *port);
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

    snprintf(cfg_topic, MAX_TOPIC, temp_cfg_topic, tcfg.name);
    Serial.println(cfg_topic);
}
