#include "ntp.h"


const char* ntpServerName = NTP_SERVER;
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
IPAddress timeServerIP; // time.nist.gov NTP server address
WiFiUDP udp;
unsigned int localPort = 2390;      // local port to listen for UDP packets
byte packetBuffer[ NTP_PACKET_SIZE ]; //buffer to hold incoming and outgoing packets


unsigned long bootTime = 0;


void setupNtp() {

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  getTime();
  
}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void printTime(unsigned long date) {

    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    
    // subtract seventy years:
    unsigned long epoch = date - seventyYears;
   
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(date);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    
    // print Unix time:
    Serial.println(epoch);

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
}


unsigned long getTime() {

  unsigned long secsSince1900 = 0;

  if (bootTime == 0 && getWifiStatus() == WL_CONNECTED) {
  	
	//get a random server from the pool
  	WiFi.hostByName(ntpServerName, timeServerIP); 

  	sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  	// wait to see if a reply is available

  	unsigned long starttime = millis();
  	unsigned long currtime = starttime;
  	
  	while(currtime - starttime < NTP_TIMEOUT) {
  	  
  	  int cb = udp.parsePacket();
  	  if (cb) {
  	    //Serial.print("packet received, length=");
  	    Serial.println(cb);
  	    // We've received a packet, read the data from it
  	    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  	    //the timestamp starts at byte 40 of the received packet and is four bytes,
  	    // or two words, long. First, esxtract the two words:

  	    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  	    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  	    // combine the four bytes (two words) into a long integer
  	    // this is NTP time (seconds since Jan 1 1900):
  	    secsSince1900 = highWord << 16 | lowWord;
  	    break;
  	  }
  	  currtime = millis();
  	}

  	if (! secsSince1900) {
  	  Serial.println("NTP TIMEOUT");
  	}
    else {
        bootTime = secsSince1900;
    }
  }
  else {
	  secsSince1900 = bootTime + millis() / 1000;
  }
  
  return secsSince1900;
}
