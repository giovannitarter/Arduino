// Control pins

#include "parallel_print_server.h"


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



void setup()
{
  Serial.begin(115200);
  // Start the printer and the serial port
  Serial.println("BOOT");

  delay(1000);

  start();
  reset();
}



void loop() 
{
  
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
