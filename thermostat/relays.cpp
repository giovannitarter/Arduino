#include <Arduino.h>
#include "relays.h"



void setupRelay() {
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
}


void relayOn(char i) {
  digitalWrite(i, HIGH);
}


void relayOff(char i) {
  digitalWrite(i, LOW);
}



