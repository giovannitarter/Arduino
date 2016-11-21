#include <Arduino.h>
#include "pinio.h"


void pinOut(uint8_t pin) {
  pinMode(pin, OUTPUT);
}


void pinIn(uint8_t pin) {
  pinMode(pin, INPUT);
}


char getPinState(uint8_t pin) {
  return digitalRead(pin);
}


void togglePin(uint8_t pin) {
  if (digitalRead(pin) == HIGH) {
    digitalWrite(pin, LOW);
  }
  else {
    digitalWrite(pin, HIGH);
  }
}


void setPin(uint8_t pin) {
  digitalWrite(pin, HIGH);  
}


void clearPin(uint8_t pin) {
  digitalWrite(pin, HIGH);  
}

