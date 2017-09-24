#include "parport.h"


void shiftdata(uint8_t c) {

    uint8_t pin;
    digitalWrite(CLK_PIN, LOW);  
    delayMicroseconds(DELAY_CLK);
    
    digitalWrite(DATA_PIN, LOW);
    delayMicroseconds(DELAY_CLK);
    digitalWrite(CLK_PIN, LOW);
    delayMicroseconds(DELAY_CLK);
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(DELAY_CLK);
    
    digitalWrite(DATA_PIN, LOW);
    delayMicroseconds(DELAY_CLK);
    digitalWrite(CLK_PIN, LOW);
    delayMicroseconds(DELAY_CLK);
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(DELAY_CLK);

    for (int i = 7; i >= 0; i--) {
    
        pin = bitRead(c, i);
        digitalWrite(DATA_PIN, pin);
        delayMicroseconds(DELAY_CLK);
        digitalWrite(CLK_PIN, LOW);
        delayMicroseconds(DELAY_CLK);
        digitalWrite(CLK_PIN, HIGH);
        delayMicroseconds(DELAY_CLK);
  
  }

  digitalWrite(DATA_PIN, LOW);
  delayMicroseconds(DELAY_CLK);
  digitalWrite(CLK_PIN, LOW);
  delayMicroseconds(DELAY_CLK);
}


void writebyte(uint8_t character) {

    unsigned long stime, ctime;
    
    digitalWrite(STROBE_PIN, HIGH);
    
    writeshr(character);
    delayMicroseconds(8);

    stime = millis();
    while(digitalRead(BUSY_PIN) == HIGH) {

        ctime = millis();
        if (ctime - stime > BUSY_TIMEOUT) {
            Serial.println("TIMEOUT2");
            break;
        }
    }

    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(STROBE_PIN, HIGH);
    delayMicroseconds(2);

    stime = millis();
    while(digitalRead(BUSY_PIN) == HIGH) {

        ctime = millis();
        if (ctime - stime > BUSY_TIMEOUT) {
            Serial.println("TIMEOUT1");
            break;
        }
    }

}


void writeshr(uint8_t chr) {
   
    
    digitalWrite(LATCH_PIN, LOW);
    delayMicroseconds(SHR_DELAY);
    
    shiftdata(chr);
    delayMicroseconds(SHR_DELAY);
    
    digitalWrite(LATCH_PIN, HIGH); 
    delayMicroseconds(SHR_DELAY);
    digitalWrite(LATCH_PIN, LOW);
    delayMicroseconds(SHR_DELAY);

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
   
}


void setup_parport() {

    pinMode(LATCH_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    
    pinMode(STROBE_PIN, OUTPUT);
    pinMode(BUSY_PIN, INPUT); 
    
    digitalWrite(STROBE_PIN, HIGH);

}


