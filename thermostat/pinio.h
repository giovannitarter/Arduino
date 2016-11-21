#ifndef _CONFIG_H_RELAYS_
#define _CONFIG_H_RELAYS_

void pinOut(uint8_t pin);
void pinIn(uint8_t pin);
char getPinState(uint8_t pin);
void togglePin(uint8_t pin);
void setPin(uint8_t pin);
void clearPin(uint8_t pin);

#endif
