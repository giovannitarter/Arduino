#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "configuration.h"
#include "otaupdates.h"

void checkOTA() {

    HTTPUpdateResult ret;
    Serial.println("checking for updates");
    ret = ESPhttpUpdate.update(OTA_ADDRESS,
            OTA_PORT,
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
            break;
        
        default:
            Serial.println("ERROR OTA");
            break;
    }
}
