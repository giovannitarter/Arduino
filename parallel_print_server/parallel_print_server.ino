// Control pins

#include <ESP8266WiFi.h>
#include "parallel_print_server.h"

#define MAX_SRV_CLIENTS 1
#define PORT 9100

const char* ssid = "GNET";
const char* password = "aqua@332345678";

WiFiServer server(PORT);
WiFiClient serverClients[MAX_SRV_CLIENTS];


void shiftdata(uint8_t c) {

  uint8_t pin;
  digitalWrite(CLK_PIN, LOW);

  for (int i = 7; i >= 0; i--) {
    
    pin = bitRead(c, i);
    digitalWrite(DATA_PIN, pin);
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(CLK_PIN, HIGH);
  }

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);
}


void writebyte(uint8_t character) {
    
    writeshr(character);
    
    while(digitalRead(BUSY_PIN) == HIGH);

    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(STROBE_PIN, HIGH);

    // Wait for the printer to finish
    while(digitalRead(BUSY_PIN) == HIGH);

}


void writeshr(uint8_t chr) {
   
    #define SHR_DELAY 0
    
    digitalWrite(LATCH_PIN, LOW);
    //shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, chr); //Send the data

    shiftdata(chr);
    
    digitalWrite(LATCH_PIN, HIGH);
    digitalWrite(LATCH_PIN, LOW);

}


void writeline() {
    
    int j;
    j = 0;

    for(uint8_t i='a'; i<='z'; i++) {
        writebyte(i);
        j++;
    }
    for(j; j<42; j++) {
        writebyte(' ');
    }
    //writebyte('\n');
    //writebyte('\r');
}


void start() {
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);

  pinMode(STROBE_PIN, OUTPUT);
  pinMode(BUSY_PIN, INPUT); 
  //pinMode(NACK_PIN, INPUT);

  digitalWrite(STROBE_PIN, HIGH);

  writebyte(0);
}


void reset() 
{
  writebyte(ASCII_ESC);
  writebyte('E'); 
  writebyte(ASCII_NEWLINE);

  
  writebyte(ASCII_ESC);
  writebyte('D'); // Set tab stops...
  writebyte(4);
  writebyte(8);
  writebyte(12);
  writebyte(16); // ...every 4 columns,
  writebyte(20); 
  writebyte(24);
  writebyte(28);
  writebyte(0); // 0 marks end-of-list.
  writebyte(0);
  
}


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
    //server.setNoDelay(true);
  
    Serial.print("Ready! IP:");
    Serial.println(WiFi.localIP());
    Serial.print("Port: ");
    Serial.println(PORT);
}


void loopServer() {
    
    int bavail = 0;
    int j = 0;
    char * input = 0;

    WiFiClient client = server.available();
    // wait for a client (web browser) to connect
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
                    //Serial.print(input[j], HEX);
                    //Serial.print(" ");
                }
                input[j] = 0;
                Serial.println("");
            
                /* 
                if (input[0] == 'a') { 
                    writeline();    
                }
                */

                for(j = 0; j < bavail + 1; j++) {
                    writebyte(input[j]);
                }

                free(input);
            }
        }

        // close the connection:
        client.stop();
        Serial.println("[Client disonnected]");
    }

//    //check if there are any new clients
//    if (server.hasClient()){
//      for(i = 0; i < MAX_SRV_CLIENTS; i++){
//        //find free/disconnected spot
//        if (!serverClients[i] || !serverClients[i].connected()){
//          if(serverClients[i]) {
//              serverClients[i].stop();
//          }
//          serverClients[i] = server.available();
//          Serial.print("New client: "); 
//          Serial.print(i);
//          continue;
//        }
//      }
//      //no free/disconnected spot so reject
//      WiFiClient serverClient = server.available();
//      serverClient.stop();
//    }
//    //check clients for data
//    for(i = 0; i < MAX_SRV_CLIENTS; i++){
//      if (serverClients[i] && serverClients[i].connected()){
//      }
//    }
}


void setup()
{
    Serial.begin(115200);
    
    start();
    reset();
    
    // Start the printer and the serial port
    delay(2000);
    Serial.println("\n\r\n\nBOOT");

    setupServer();

}



void loop() 
{

    loopServer(); 
 
    /*

    Serial.println("waiting for input:");
 
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
            writeline();    
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
