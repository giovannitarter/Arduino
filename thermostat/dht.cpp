//
// FILE: dht.cpp
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET: http://www.micro4you.com/files/sensor/DHT11.pdf
//
// HISTORY:
// George Hadjikyriacou - Original version (??)
// Mod by SimKard - Version 0.2 (24/11/2010)
// Mod by Rob Tillaart - Version 0.3 (28/03/2011)
// + added comments
// + removed all non DHT11 specific code
// + added references
// Mod by Rob Tillaart - Version 0.4 (17/03/2012)
// + added 1.0 support
// Mod by Rob Tillaart - Version 0.4.1 (19/05/2012)
// + added error codes
//

#include "dht.h"


DHT::DHT(char pin_p, char type_p) {
    pin = pin_p;
    type = type_p;
    lastReadTime = millis();
    lastRes = DHTLIB_ERROR_NOT_INIT;    

    hum = 0;
	temp = 0;
    hum_dec = 0;
    temp_dec = 0;
}


void DHT::toggle_type()
{
    if (type == SENS_DHT12) {
        type = SENS_DHT22;
    }
    else if (type == SENS_DHT22) {
        type = SENS_DHT12;
    }
    else {
        type = SENS_DHT12;
    }
}


// Return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
char DHT::read_dht()
{
    
    unsigned long ctime = millis();

    if (
            (ctime - lastReadTime) < MIN_READ_INTERVAL 
            && lastRes != DHTLIB_ERROR_NOT_INIT
       ) 
    {
        return lastRes;
    }

	// BUFFER TO RECEIVE
    char res = DHTLIB_OK;
    uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;

    uint8_t sum;
    uint16_t tmp;
	
    int16_t loopCnt;
	unsigned long pmillis, pmicros;
    int i;

	// EMPTY BUFFER
	for (i=0; i< 5; i++) {
        bits[i] = 0;
    }

	// REQUEST SAMPLE
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	delay(18);
	digitalWrite(pin, HIGH);
	delayMicroseconds(40);
	pinMode(pin, INPUT);


	//Wait that dht pin is low
    pmillis = millis();
    while(digitalRead(pin) == LOW) {
        if (millis() - pmillis > DTH_READ_TIMEOUT) {
            res = DHTLIB_ERROR_TIMEOUT;
            break;
        }
    } 
    
	//Wait that dht pin becomes high
    if (res == DHTLIB_OK) {
        pmillis = millis();
        while(digitalRead(pin) == HIGH) {
            if (millis() - pmillis > DTH_READ_TIMEOUT) {
                res = DHTLIB_ERROR_TIMEOUT;
                break;
            }
        } 
    }

    if (res == DHTLIB_OK) {
	    // READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	    for (i=0; i<40; i++)
	    {
            
            pmillis = millis();
            while(digitalRead(pin) == LOW) {
                if (millis() - pmillis > DTH_READ_TIMEOUT) {
                    res = DHTLIB_ERROR_TIMEOUT;
                    break;
                }
            } 
	
	    	pmicros = micros();
            pmillis = millis();
            while(digitalRead(pin) == HIGH) {
                if (millis() - pmillis > DTH_READ_TIMEOUT) {
                    res = DHTLIB_ERROR_TIMEOUT;
                    break;
                }
            } 

	    	if ((micros() - pmicros) > 40) {
                bits[idx] |= (1 << cnt);
            }
	    	
            if (cnt == 0) {
	    		cnt = 7;    // restart at MSB
	    		idx++;      // next byte!
	    	}
	    	else cnt--;
	    }
    
        sum = bits[0] + bits[1] + bits[2] + bits[3];  
	    if (bits[4] != sum) {
            lastRes = DHTLIB_ERROR_CHECKSUM;
            res = DHTLIB_ERROR_CHECKSUM;
        }
    }
   
    if (res == DHTLIB_OK) {
    
        if (type == SENS_DHT12) { 
	        hum = bits[0];
            hum_dec = bits[1]; 
	        temp = bits[2];
	        temp_dec = bits[3]; 
        }
        else if (type == SENS_DHT22) {
            tmp = bits[0];
            tmp = tmp << 8;
            tmp += bits[1];
            hum = tmp / 10;
            hum_dec = tmp % 10;
          
            tmp = bits[2];
            tmp = tmp << 8;
            tmp += bits[3];
            temp = tmp / 10;
            temp_dec = tmp % 10;
        }
        else {
            res = DHTLIB_ERROR_CFG;
        }
    }

    lastReadTime = millis();
    lastRes = res;
    return res;
}
