#include <hid_mouse.h>
#include <usbconfig.h>


void setup() {
    Serial.begin(115200);
    Serial.println("Setup");

    //Serial.println("VUSB setup onmsg callback");
    //VUsbHid.OnMsg(OnMidiMessage);
    
    Serial.println("VUSB begin");
    VUsbHid.begin(false);
    
}


void loop() {
  VUsbHid.loop();
}
