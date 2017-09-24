// Control pins

#include <ESP8266WiFi.h>

#include "parallel_print_server.h"
#include "parport.h"

#include "wificreds.h"

#define MAX_SRV_CLIENTS 1
#define PORT 9100

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

WiFiServer server(PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];


void setupServer() {

    Serial.print("\nConnecting to "); 
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("connected");
  
    server.begin();
    server.setNoDelay(true);
  
    Serial.print("Ready! IP:");
    Serial.println(WiFi.localIP());
    Serial.print("Port: ");
    Serial.println(PORT);
}


void loopServer() {
    
    int bavail = 0;
    int j = 0;
    char * input = 0;
    WiFiClient client;

    client = server.available();    
    if (client)
    {
    
        Serial.println("\n[Client connected]");
        while (client.connected())
        {
            bavail = client.available();

            // read line by line what the client (web browser) is requesting
            if (bavail)
            {
                Serial.print("available: ");
                Serial.println(bavail);
                Serial.println("data:");
                input = (char *) malloc(bavail + 1);
 
                for(j = 0; j<bavail; j++) {
                    input[j] = client.read();
                    Serial.print(input[j], HEX);
                    Serial.print(" ");
                }
                input[j] = 0;
                Serial.println("");
            
                for(j = 0; j < bavail + 1; j++) {
                    //writebyte(input[j]);
                }

                free(input);
            }
        }

        // close the connection:
        client.stop();
        Serial.println("[Client disonnected]");
    }
}


void setup()
{
    Serial.begin(115200);
    
    // Start the printer and the serial port
    delay(2000);
    Serial.println("\n\r\n\nBOOT");

    setupServer();
    setup_parport();
}


void loop() 
{
    loopServer(); 
 
    
/*
    Serial.println("\nwaiting for input:");
 
    while (Serial.available() == 0) {
    }

    int bavail;
    char incomingByte;
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
  */  

    /*
    char incomingByte;
    if (Serial.available()) {
        
        delay(1);
        while(Serial.available()) {
          incomingByte = Serial.read();
          writebyte(incomingByte);
        }
        writeshr(0);
    
    }
    */
}
