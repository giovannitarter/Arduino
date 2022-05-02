#ifndef __GVC_GDEFINES__
#define __GVC_GDEFINES__

#define CAP_CHARGE_TIME 5000 //msec
#define SOLENOID_PULSE_TIME 75 //msec

#define BOOT_DELAY 1 //sec

#define EVT_TOLERANCE 10 //sec
#define SLEEP_MAX 30 //sec

#define OP_NONE  0x00
#define OP_SKIP  0x10
#define OP_OPEN  0x11
#define OP_CLOSE 0x12


uint8_t decode_action(uint8_t msg_action);
size_t read_schedule(char * buffer, size_t buflen);


#endif


