#ifndef __hidmouse_h_
#define __hidmouse_h_

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

#include "vusb/usbdrv.h"
#include <util/delay.h>     /* for _delay_ms() */


#ifdef __cplusplus
extern "C"
#endif 

void advanceCircleByFixedAngle(void);

class HidMouse {
	
    private:
		bool _calibrate_osc = false;
		bool _optimalize_osc = false;
		
	public:
		HidMouse (void);
		
        //void OnMsg(void (*onMsgCallback)(TFMidiMessage));
		void OnUSBReset(void);
		void calibrateOSC(void);
		
		void begin(bool calibrate_osc = false);
    void loop(void);
		//void refresh();
		//void read(uchar *data, uchar len);
		
		//void write(TFMidiMessage msg);
		//void write(byte *buffer, byte size);
};

extern HidMouse VUsbHid;

#endif // __hidmouse_h_
