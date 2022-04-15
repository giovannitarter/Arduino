/** Ds1302.cpp
 *
 * Ds1302 class.
 *
 * @version 1.0.3
 * @author Rafa Couto <caligari@treboada.net>
 * @license GNU Affero General Public License v3.0
 * @see https://github.com/Treboada/Ds1302
 *
 */

#include <ESP8266WiFi.h>
#include "Ds1302.h"


#define REG_SECONDS           0x80
#define REG_MINUTES           0x82
#define REG_HOUR              0x84
#define REG_DATE              0x86
#define REG_MONTH             0x88
#define REG_DAY               0x8A
#define REG_YEAR              0x8C
#define REG_WP                0x8E
#define REG_BURST             0xBE


Ds1302::Ds1302(uint8_t pin_ena, uint8_t pin_clk, uint8_t pin_dat)
{
    _pin_ena = pin_ena;
    _pin_clk = pin_clk;
    _pin_dat = pin_dat;
    _dat_direction = INPUT;
}


void Ds1302::init()
{
    pinMode(_pin_ena, OUTPUT);
    pinMode(_pin_clk, OUTPUT);
    pinMode(_pin_dat, _dat_direction);

    digitalWrite(_pin_ena, LOW);
    digitalWrite(_pin_clk, LOW);
}


bool Ds1302::isHalted()
{
    _prepareRead(REG_SECONDS, 1);
    uint8_t seconds = _readByte();
    _end();
    return (seconds & 0b10000000);
}


void Ds1302::getDateTime(struct tm * dt)
{
    _prepareRead(REG_BURST, 1);
    dt->tm_sec  = _bcd2dec(_readByte() & 0b01111111);
    dt->tm_min  = _bcd2dec(_readByte() & 0b01111111);
    dt->tm_hour = _bcd2dec(_readByte() & 0b00111111);
    dt->tm_mday = _bcd2dec(_readByte() & 0b00111111);
    dt->tm_mon  = _bcd2dec(_readByte() & 0b00011111);
                  _bcd2dec(_readByte() & 0b00000111);
    dt->tm_year = _bcd2dec(_readByte() & 0b01111111) + 100;
    _end();

    dt->tm_isdst = 0;
    dt->tm_yday = 0;
    dt->tm_wday = 0;
}


void Ds1302::setDateTime(struct tm * dt)
{
    _prepareWrite(REG_WP, 1);
    _writeByte(0b00000000);
    _end();

    _prepareWrite(REG_BURST, 1);
    _writeByte(_dec2bcd(dt->tm_sec  % 60 ));
    _writeByte(_dec2bcd(dt->tm_min  % 60 ));
    _writeByte(_dec2bcd(dt->tm_hour % 24 ));
    _writeByte(_dec2bcd(dt->tm_mday % 32 ));
    _writeByte(_dec2bcd(dt->tm_mon  % 13 ));
    _writeByte(_dec2bcd(dt->tm_wday % 8  ));
    _writeByte(_dec2bcd(dt->tm_year % 100));
    _writeByte(0b10000000);
    _end();
}
        
void Ds1302::writeRam(uint8_t address, uint8_t value) {
   
    address <<= 1;
    address &= 0b00111110;

    _prepareWrite(REG_WP, 1);
    _writeByte(0b00000000);
    _end();
    
    _prepareWrite(address, 0);
    _writeByte(value);
    _end();
}


void Ds1302::readRam(uint8_t address, uint8_t * value) {
    
    address <<= 1;
    address &= 0b00111110;
    
    _prepareRead(address, 0);
    *value = _readByte();
    _end();
}


void Ds1302::halt()
{
    _prepareWrite(REG_SECONDS, 1);
    _writeByte(0b10000000);
    _end();
}


void Ds1302::_prepareRead(uint8_t address, uint8_t clock)
{
    uint8_t command;
    if (clock) {
        command = 0b10000001;
    }
    else {
        command = 0b11000001;
    }
    _setDirection(OUTPUT);
    digitalWrite(_pin_ena, HIGH);
    command |= address;
    _writeByte(command);
    _setDirection(INPUT);
    
}


void Ds1302::_prepareWrite(uint8_t address, uint8_t clock)
{
    uint8_t command;
    if (clock) {
        command = 0b10000000;
    }
    else {
        command = 0b11000000;
    }
    
    _setDirection(OUTPUT);
    digitalWrite(_pin_ena, HIGH);
    command |= address;
    _writeByte(command);
    
}


void Ds1302::_end()
{
    digitalWrite(_pin_ena, LOW);
}


uint8_t Ds1302::_readByte()
{
    uint8_t byte = 0;

    for(uint8_t b = 0; b < 8; b++)
    {
        if (digitalRead(_pin_dat) == HIGH) byte |= 0x01 << b;
        _nextBit();
    }

    return byte;
}


void Ds1302::_writeByte(uint8_t value)
{
    for(uint8_t b = 0; b < 8; b++)
    {
        digitalWrite(_pin_dat, (value & 0x01) ? HIGH : LOW);
        _nextBit();
        value >>= 1;
    }
}

void Ds1302::_nextBit()
{
        digitalWrite(_pin_clk, HIGH);
        delayMicroseconds(1);

        digitalWrite(_pin_clk, LOW);
        delayMicroseconds(1);
}


void Ds1302::_setDirection(int direction)
{
    if (_dat_direction != direction)
    {
        _dat_direction = direction;
        pinMode(_pin_dat, direction);
    }
}


uint8_t Ds1302::_dec2bcd(uint8_t dec)
{
    return ((dec / 10 * 16) + (dec % 10));
}


uint8_t Ds1302::_bcd2dec(uint8_t bcd)
{
    return ((bcd / 16 * 10) + (bcd % 16));
}


