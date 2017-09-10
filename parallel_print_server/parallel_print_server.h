#ifndef PARALLEL_PRINT_SERVER_H
#define PARALLEL_PRINT_SERVER_H


#define ASCII_ESC 27
#define ASCII_NEWLINE 10

//shift register
//Pin connected to STR(pin 1) of HEF4094
#define LATCH_PIN 13 

//Pin connected to D(pin 2) of HEF4094
#define DATA_PIN 12 

//Pin connected to CP(pin 3) of HEF4094
#define CLK_PIN 14



//parallel port
#define STROBE_PIN 4
#define BUSY_PIN 5
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

void cut();
void feed();
void shiftdata(uint8_t c);
void writebyte(uint8_t character);
void writeshr(uint8_t character);
void start();
void reset();

void loopServer();
void setupServer();

#endif
