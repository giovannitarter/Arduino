/** Ds1302.h
 *
 * Ds1302 class.
 *
 * @version 1.0.3
 * @author Rafa Couto <caligari@treboada.net>
 * @license GNU Affero General Public License v3.0
 * @see https://github.com/Treboada/Ds1302
 *
 */

#ifndef _DS_1302_H
#define _DS_1302_H

#include <stdint.h>

class Ds1302
{
    public:

        /**
         * Constructor (pin configuration).
         */
        Ds1302(uint8_t pin_ena, uint8_t pin_clk, uint8_t pin_dat);

        /**
         * Initializes the DW1302 chip.
         */
        void init();

        /**
         * Returns when the oscillator is disabled.
         */
        bool isHalted();

        /**
         * Stops the oscillator.
         */
        void halt();

        /**
         * Returns the current date and time.
         */
        void getDateTime(struct tm * dt);

        /**
         * Sets the current date and time.
         */
        void setDateTime(struct tm * dt);

        void writeRam(uint8_t address, uint8_t value);
        void readRam(uint8_t address, uint8_t * value);

    private:

        uint8_t _pin_ena;
        uint8_t _pin_clk;
        uint8_t _pin_dat;

        void _prepareRead(uint8_t address, uint8_t clock);
        void _prepareWrite(uint8_t address, uint8_t clock);
        void _end();

        int _dat_direction;
        void _setDirection(int direction);

        uint8_t _readByte();
        void _writeByte(uint8_t value);
        void _nextBit();

        uint8_t _dec2bcd(uint8_t dec);
        uint8_t _bcd2dec(uint8_t bcd);
};

#endif // _DS_1302_H

