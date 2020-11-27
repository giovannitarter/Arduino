#ifndef __hiddata_h_
#define __hiddata_h_

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

#include "vusb/usbdrv.h"
#include <util/delay.h>     /* for _delay_ms() */

#ifdef __cplusplus
extern "C" {
#endif 

class HidData {
    
    private:
        bool _calibrate_osc = false;
        bool _optimalize_osc = false;

    public:
        HidData (void);
        
        //void OnMsg(void (*onMsgCallback)(TFMidiMessage));
        void OnUSBReset(void);
        void calibrateOSC(void);
        
        void begin(bool calibrate_osc = false);
        void loop(void);
		
        void setReadCallback(uchar (*hidReadCallback)(uchar *data, uchar len));
        void setWriteCallback(uchar (*hidWriteCallback)(uchar *data, uchar len));
		
        uchar (*_hidReadCallback)(uchar *data, uchar len); 
		uchar (*_hidWriteCallback)(uchar *data, uchar len); 
        
        //void refresh();
        //void read(uchar *data, uchar len);
        
        //void write(TFMidiMessage msg);
        //void write(byte *buffer, byte size);
};

#ifdef __cplusplus
}
#endif 

extern HidData VUsbHid;

#endif // __hiddata_h_
