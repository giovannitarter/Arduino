#ifndef PARPORT_H
#define PARPORT_H

#include <Arduino.h>

#define ASCII_ESC 27
#define ASCII_NEWLINE 10


#define DELAY_CLK 4 //us
#define BUSY_TIMEOUT 2 //ms
#define SHR_DELAY 4 //us


//shift register
//Pin connected to STR(pin 1) of HEF4094
#define LATCH_PIN 14 

//Pin connected to D(pin 2) of HEF4094
#define DATA_PIN 12 

//Pin connected to CP(pin 3) of HEF4094
#define CLK_PIN 13



//parallel port
#define STROBE_PIN 5
#define BUSY_PIN 4
//#define NACK_PIN 4


/*
//shift register
//Pin connected to STR(pin 1) of HEF4094
#define LATCH_PIN 2

//Pin connected to D(pin 2) of HEF4094
#define DATA_PIN 3 

//Pin connected to CP(pin 3) of HEF4094
#define CLK_PIN 4

//parallel port
#define STROBE_PIN 5
#define BUSY_PIN 6
//#define NACK_PIN 4
*/

void shiftdata(uint8_t c);
void writebyte(uint8_t character);
void writeshr(uint8_t character);
void setup_parport();




#endif
