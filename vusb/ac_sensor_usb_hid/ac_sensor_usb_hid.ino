#include <hid_data.h>
#include <usbconfig.h>


HidData VUsbHid = HidData();

uchar writeCallback(uchar * data, uchar len) {
  
  Serial.println("writeCallback");
  Serial.println(len);

  int i;
  for (i=0; i<len; i++) {
    if (i % 16 == 0) Serial.println("");
    Serial.print(data[i]);
    Serial.print(" ");
  }
  
  return 0;
}


uchar readCallback(uchar * data, uchar len) {
  
  //Serial.println("readCallback");

  int i;
  for (i=0; i<len; i++) {
    data[i] = i;
  }
  
  return 0;
}


void setup() {
  
    Serial.begin(115200);
    Serial.println("Setup");

    Serial.println("VUSB begin");
    VUsbHid.begin(false);
    
    VUsbHid.setReadCallback(readCallback);
    VUsbHid.setWriteCallback(writeCallback);
    
}


void loop() {
  VUsbHid.loop();
}
