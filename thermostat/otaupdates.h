#ifndef _CONFIG_H_OTAUPDATES_
#define _CONFIG_H_OTAUPDATES_

void IpAddressToStr(IPAddress * addr, char * res, int len);
char checkOTA(char * addr, uint16_t port);

#endif
