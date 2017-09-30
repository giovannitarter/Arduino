#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "thermostat.h"
#include "otaupdates.h"


char checkOTA(char * addr, uint16_t port) {

    char res;
    
    HTTPUpdateResult ret;
    Serial.println("\nchecking for updates");
    
    res = 0;
    ret = ESPhttpUpdate.update(
            addr,
            port,
            OTA_LOCATION,
            FW_VERSION);

    switch(ret) {
        
    case HTTP_UPDATE_FAILED:
            Serial.printf(
                "HTTP_UPDATE_FAILD Error (%d): %s", 
                ESPhttpUpdate.getLastError(), 
                ESPhttpUpdate.getLastErrorString().c_str());
                break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

         case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            res = 1;
            break;
        
        default:
            Serial.println("ERROR OTA");
            break;
    }
    return res;
}
