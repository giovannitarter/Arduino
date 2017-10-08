// Control pins

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "parallel_print_server.h"
#include "parport.h"

#include "wificreds.h"

#define MAX_SRV_CLIENTS 1
#define PORT 9100

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

WiFiServer server(PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];


bool checkConnection() {
    
    bool res = false;

    if (WiFi.status() == WL_CONNECTED) {
        res = true;
    }
    
    return res;
}


bool attemptConnection() {

    bool res;
    res = true;    

    server.stop();

    Serial.printf("Connecting to %s\n", ssid); 
    WiFi.disconnect(); 
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("connected");
    
    if (!MDNS.begin("esp8266")) {
        Serial.println("Error setting up MDNS responder!");
    }
    else {
        Serial.println("mDNS responder started");
        MDNS.addService("printer", "tcp", 9100); 
    }
    
    server.begin();
    //server.setNoDelay(true);
    server.setNoDelay(false);
  
    Serial.printf("Ready! ADDR: %s:%d\n", 
        WiFi.localIP().toString().c_str(), 
        PORT
    );
    
    return res;
}


void loopServer() {
    
    int b_avail = 0, j = 0, b_read = 0;
    static char input[MAX_PACKET];
    WiFiClient client;

    if (checkConnection == false) {
        while (attemptConnection() == false);
    }

    client = server.available();    
    if (client)
    {
        Serial.println("[Client connected]");
        while (client.connected())
        {
            b_avail = client.available();

            // read line by line what the client (web browser) is requesting
            if (b_avail > 0)
            {
                memset(input, 0, MAX_PACKET);
                Serial.printf("bytes available: %d\n", b_avail);
                b_read = client.read((uint8_t *)input, b_avail);
                Serial.printf("bytes read: %d\n", b_read);
                for(j = 0; j < b_read; j++) {
                    writebyte(input[j]);
                }
            }
        }

        // close the connection:
        client.stop();
        Serial.println("[Client disconnected]");
    }
}


void test_loop() {
    
    int bavail;
    char incomingByte;
    
    Serial.println("\nwaiting for input:");
 
    while (Serial.available() == 0) {
        yield();
    }

    bavail = Serial.available();

    if (bavail > 0) {
        
        // read the incoming byte:
        while (Serial.available()) {
            incomingByte = Serial.read();
        }
       
        if (incomingByte == 'a') { 
            Serial.println("Received!");  
            writeline();    
            Serial.println("Line finished!");  
        }
    }
}


void setup()
{
    Serial.begin(115200);
    
    // Start the printer and the serial port
    delay(2000);
    Serial.println("\n\nBOOT");

    setup_parport();
    attemptConnection();
}


void loop() 
{
    loopServer(); 
    //test_loop();
}
