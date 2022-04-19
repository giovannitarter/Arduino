#include <ESP8266WiFi.h>
#include <coredecls.h>
#include <sntp.h>
#include "LittleFS.h"

#include "pb_encode.h"

#include "garden_valve_controller.h"
#include "Ds1302.h"
#include "weekly_calendar.h"
#include "solenoid_driver.h"

//D1 GPIO5
//D2 GPIO4
//D5 GPIO14
//D6 GPIO12
//D7 GPIO13

#define PIN_CLK 5
#define PIN_DIO 4
#define PIN_CE  13

#define PIN_SU   14 //D5
#define PIN_STBY 12 //D6

#define PIN_H_A1 5  //D1
#define PIN_H_A2 4  //D2

#define TIME_MIN 1640995200

#define ADDR_CNT 0x01

#define ADDR_CHECK 0x02
#define VAL_CHECK 0b0101010101

#define ADDR_NEXTOP 0x03

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

//Globals
int dir;
const int VALVE_DELAY = BOOT_DELAY + CAP_CHARGE_TIME + SOLENOID_PULSE_TIME;

#define TZ "CET-1CEST,M3.5.0/2,M10.5.0/3"

Ds1302 rtc = Ds1302(PIN_CE, PIN_CLK, PIN_DIO);
WeeklyCalendar wk = WeeklyCalendar();
SolenoidDriver soldrv = SolenoidDriver();


time_t now;
struct tm tm_now;


//Callback called when NTP acquires time
void time_is_set(bool from_sntp) {

  time_t now;
  //Serial.print("time is set - ");
  //Serial.println(from_sntp);

  if (from_sntp) {

    now = time(nullptr);
    rtc_set_time(now);

    Serial.print("Writing time to rtc: ");
    Serial.println(now);

    now = rtc_get_time();
    Serial.print("Verify: ");
    Serial.println(now);
  }
}


void system_set_time(time_t curr_time) {
    timeval tv = {curr_time, 0};
    settimeofday(&tv, nullptr);
    return;
}


void rtc_set_time(time_t time_p) {

    struct tm tmp;

    gmtime_r(&time_p, &tmp);

    //printTm("setting to rtc: ", &tmp);
    rtc.setDateTime(&tmp);
    rtc.writeRam(ADDR_CNT, 0);
}


time_t rtc_get_time() {

    time_t res, offset;
    struct tm tmp, epoch;

    rtc.getDateTime(&tmp);

    //printTm("get from rtc: ", &tmp);

    memset(&epoch, 0, sizeof(struct tm));
    epoch.tm_mday = 2;
    epoch.tm_year = 70;
    offset = mktime(&epoch) - 60*60*24;

    // Now we are ready to convert tm to time_t in UTC.
    // as mktime adds timezone, subtracting offset(=timezone) gives us the right result
    res = mktime(&tmp) - offset;

    return(res);
}


void set_time_boot(int config) {

    //RTC stores time in UTC

    uint8_t need_sync;
    time_t rtc_time, now;

    settimeofday_cb(time_is_set);

    configTime(0, 0, "pool.ntp.org");
    setenv("TZ", TZ, 3);
    tzset();

    uint8_t cnt;
    rtc.readRam(ADDR_CNT, &cnt);
    Serial.printf("cnt: %d\n\r", cnt);
    rtc.writeRam(ADDR_CNT, cnt + 1);

    rtc_time = rtc_get_time();

    Serial.print("rtc_time: ");
    Serial.println((unsigned int)rtc_time);

    //rtc_status = rtc_failed();
    //Serial.print("rtc_status: ");
    //Serial.println(rtc_status);

    need_sync = rtc_time < TIME_MIN || config;

    Serial.print("need_sync: ");
    Serial.println(need_sync);

    if (need_sync) {

        Serial.println("Error on RTC, sync from the internet");

        now = time(nullptr);
        wk.print_time_t("localtime:", now, 0);
        Serial.println();

        WiFi.mode(WIFI_STA);
        WiFi.begin(STASSID, STAPSK);

        uint32_t timeout = millis() + 10000;
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
          if (millis() > timeout) {
            Serial.println("Connection timeout");
            break;
          }
        }

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else {
        Serial.println("RTC is ok, setting time to system clock");
        system_set_time(rtc_time);

    }

}


void write_schedule(File * f) {
    
    bool status;
    size_t len;
    Schedule msg;
    uint8_t buffer[128];

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    msg.version = 1;
    status = pb_encode(&stream, Schedule_fields, &msg);
    len = stream.bytes_written;

    Serial.printf("write len: %d\n", len);
    
    f->write((char *)buffer, len);
}



void setup() {
    
    pinMode(PIN_SU, OUTPUT);
    pinMode(PIN_STBY, OUTPUT);

    digitalWrite(PIN_SU, LOW);
    digitalWrite(PIN_STBY, LOW);
    
    rtc.init();
    soldrv.init(PIN_SU, PIN_STBY, PIN_H_A1, PIN_H_A2);

    int config = 0;

    delay(BOOT_DELAY);

    //digitalWrite(PIN_H_A1, LOW);
    //pinMode(PIN_H_A1, INPUT);
    //config = ! digitalRead(PIN_H_A1);

    Serial.begin(115200);
    Serial.println("\n\nBOOT");

    Serial.print("config: ");
    Serial.println(config);

    Serial.print("STASSID: ");
    Serial.println(STASSID);

    Serial.print("STAPSK: ");
    Serial.println(STAPSK);


    dir = 0;

    set_time_boot(config);

    LittleFS.begin();
    if (! LittleFS.exists("/schedule.txt")) {
        
        Serial.println("Writing schedule");
        
        File f = LittleFS.open("schedule.txt", "w");
        write_schedule(&f);
        f.close();
    }
    LittleFS.end();

    Serial.println("end setup\n");
}


void loop() {

    now = time(nullptr);
    Serial.print("UTC time from system:  ");
    Serial.println((unsigned int)now);

    Serial.print("UTC time from rtc:     ");
    now = rtc_get_time();
    Serial.println((unsigned int)now);

    wk.print_time_t("localtime from system: ", now, 0);

    time_t next, last, sleeptime;
    uint8_t last_op, next_op, curr_op;
    struct tm next_tm;

    Serial.println("");
    wk.next_event_r(now, 0, &curr_op, &sleeptime);

    
    switch (curr_op) {
        
        case OP_SKIP:
            break;

        case OP_OPEN:
            Serial.println("Time to open");
            soldrv.open_valve();
            break;

        case OP_CLOSE:
            Serial.println("Time to close");
            soldrv.close_valve();
            break;

        default:
            break;

    }


    Serial.printf("Will sleep for: %d\n\r", sleeptime); 
    sleeptime = sleeptime * 1e6;
    ESP.deepSleep(sleeptime);
}

