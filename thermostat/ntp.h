#ifndef _NTP_THERMOSTAT_H_
#define _NTP_THERMOSTAT_H_






unsigned long sendNTPpacket(IPAddress& address);
void setupNtp();
unsigned long getTime();
void printTime(unsigned long date);

#endif
