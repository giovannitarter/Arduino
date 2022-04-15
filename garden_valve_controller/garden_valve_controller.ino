#include <ESP8266WiFi.h>
#include <coredecls.h>
#include <sntp.h>

#include "garden_valve_controller.h"
#include "Ds1302.h"
#include "weekly_calendar.h"

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

#define TZ "CET-1CEST,M3.5.0/2,M10.5.0/3"

Ds1302 rtc = Ds1302(PIN_CE, PIN_CLK, PIN_DIO);
WeeklyCalendar wk = WeeklyCalendar(MAX_ENTRIES);


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


void close_valve() {

    pinMode(PIN_H_A1, OUTPUT);
    pinMode(PIN_H_A2, OUTPUT);
    digitalWrite(PIN_SU, HIGH);

    Serial.println("Charging...");
    delay(5000);
    Serial.println("Charged!");

    digitalWrite(PIN_H_A1, HIGH);
    digitalWrite(PIN_H_A2, LOW);

    Serial.println("Activation");
    digitalWrite(PIN_STBY, HIGH);
    delay(75);
    Serial.println("Stop");

    digitalWrite(PIN_STBY, LOW);
    digitalWrite(PIN_SU, LOW);
    digitalWrite(PIN_H_A1, LOW);
    digitalWrite(PIN_H_A2, LOW);

}


void open_valve() {

    pinMode(PIN_H_A1, OUTPUT);
    pinMode(PIN_H_A2, OUTPUT);
    digitalWrite(PIN_SU, HIGH);

    Serial.println("Charging...");
    delay(5000);
    Serial.println("Charged!");

    digitalWrite(PIN_H_A1, LOW);
    digitalWrite(PIN_H_A2, HIGH);

    Serial.println("Activation");
    digitalWrite(PIN_STBY, HIGH);
    delay(75);
    Serial.println("Stop");

    digitalWrite(PIN_STBY, LOW);
    digitalWrite(PIN_SU, LOW);
    digitalWrite(PIN_H_A1, LOW);
    digitalWrite(PIN_H_A2, LOW);

}


void setup() {
    
    pinMode(PIN_SU, OUTPUT);
    pinMode(PIN_STBY, OUTPUT);
    //pinMode(PIN_H_A1, OUTPUT);
    //pinMode(PIN_H_A2, OUTPUT);

    digitalWrite(PIN_SU, LOW);
    digitalWrite(PIN_STBY, LOW);
    //digitalWrite(PIN_H_A1, LOW);
    //digitalWrite(PIN_H_A2, LOW);
    
    rtc.init();

    int config = 0;

    delay(BOOT_DELAY * 1e3);

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
    wk.parse_schedule();

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
    wk.next_event(now, &last, &next, &last_op, &next_op);

    //Serial.printf("next_op: %X\n\r", next_op);
    //Serial.printf("last_op: %X\n\r", last_op);
   
    if (now - last < EVT_TOLERANCE) {
        curr_op = last_op;
        Serial.printf("Executing last_op: %X\n\r", curr_op);
    }
    else if (next - now < EVT_TOLERANCE) {
        curr_op = next_op;
        Serial.printf("Executing next_op: %X\n\r", curr_op);
    }
    else {
        curr_op = OP_SKIP;
    }
    
    switch (curr_op) {
        
        case OP_SKIP:
            break;

        case OP_OPEN:
            Serial.println("Time to open");
            open_valve();
            break;

        case OP_CLOSE:
            Serial.println("Time to close");
            close_valve();
            break;

        default:
            break;

    }

    now = time(nullptr);
    sleeptime = (unsigned int)next-now; 
    
    if (sleeptime > SLEEP_MAX) {
        sleeptime = SLEEP_MAX;
    }
    else if (sleeptime < EVT_TOLERANCE && curr_op != OP_SKIP) {
        sleeptime = EVT_TOLERANCE;
    }
    //rtc.writeRam(ADDR_NEXTOP, next_op);
   
    if (curr_op != OP_SKIP) { 
        sleeptime -= BOOT_DELAY;
    }

    Serial.printf("Will sleep for: %d\n\r", sleeptime);
    
    sleeptime = sleeptime * 1e6;
    ESP.deepSleep(sleeptime);
}

