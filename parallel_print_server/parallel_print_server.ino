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
    static char input[MAX_PACKET], * cptr;
    WiFiClient client;
    int total_bytes = 0;

    if (checkConnection == false) {
        while (attemptConnection() == false);
    }

    client = server.available();
    total_bytes = 0;
    cptr = input;
 
    if (client)
    {
        Serial.println("[Client connected]");
        while (client.connected() && total_bytes < MAX_PACKET - 1)
        {
            b_avail = client.available();
            Serial.printf("bytes avail: %d\n", b_avail);
                
            if (total_bytes + b_avail > MAX_PACKET) {
                b_avail = MAX_PACKET - total_bytes;
            }

            b_read = client.read((uint8_t *)cptr, b_avail);
            
            cptr += b_read;
            total_bytes += b_read;
        }

        // close the connection:
        client.stop();
        Serial.println("[Client disconnected]");
                
        Serial.printf("total bytes: %d\n", total_bytes);
        for(j = 0; j < total_bytes; j++) {
            if (j % 200 == 0) {
                yield();
            }
            writebyte(input[j]);
        }
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
