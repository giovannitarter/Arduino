// 
//    FILE: dht11.h
// VERSION: 0.4.1
// PURPOSE: DHT11 Temperature & Humidity Sensor library for Arduino
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET: http://www.micro4you.com/files/sensor/DHT11.pdf
//
//     URL: http://playground.arduino.cc/Main/DHT11Lib
//
// HISTORY:
// George Hadjikyriacou - Original version
// see dht.cpp file
// 

#ifndef dht_h
#define dht_h

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define DHT11LIB_VERSION "0.4.1"

#define DHTLIB_OK 0
#define DHTLIB_ERROR_CHECKSUM 2
#define DHTLIB_ERROR_TIMEOUT 2
#define DHTLIB_ERROR_CFG 3
#define DHTLIB_ERROR_VALUE 4
#define DHTLIB_ERROR_NOT_INIT 5

#define SENS_DHT22 4
#define SENS_DHT12 5

#define MIN_READ_INTERVAL 2500 //ms

#define DTH_READ_TIMEOUT 5 //ms

class DHT
{
    private:
    
        char pin;
        unsigned long lastReadTime; 

    public:

        DHT(char pin, char type);
        char read_dht();
        void toggle_type();
        
        char type;
        
        uint8_t hum;
	    uint8_t temp;
        uint8_t hum_dec;
        uint8_t temp_dec;

        char lastRes;
};



#endif
//
// END OF FILE
//
