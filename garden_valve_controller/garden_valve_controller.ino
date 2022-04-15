#include <ESP8266WiFi.h>
#include <coredecls.h>
#include <sntp.h>
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
#define SLEEP_MAX 20

#define ADDR_CHECK 0x02
#define VAL_CHECK 0b0101010101

#define ADDR_NEXTOP 0x03
#define OP_SKIP 0x10
#define OP_OPEN 0x11
#define OP_CLOSE 0x12

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

//Globals
int dir;

#define TZ "CET-1CEST,M3.5.0/2,M10.5.0/3"

Ds1302 rtc = Ds1302(PIN_CE, PIN_CLK, PIN_DIO);


time_t now;
struct tm tm_now;




#define PTM(w) \
  Serial.print(" " #w "="); \
  Serial.print(tm->tm_##w);

void printTm(const char* what, const tm* tm) {
  Serial.print(what);
  PTM(isdst); PTM(yday); PTM(wday);
  PTM(year);  PTM(mon);  PTM(mday);
  PTM(hour);  PTM(min);  PTM(sec);
  Serial.println("");
}


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
        printTm("localtime:", localtime(&now));
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


ws_entry schedule[MAX_ENTRIES];
time_t on_events[MAX_ENTRIES];
time_t off_events[MAX_ENTRIES];
int events_nr;

void parse_schedule() {
    
    events_nr = 0;

    memset(schedule, 0, sizeof(ws_entry) * MAX_ENTRIES);
    
    //next monday at 11.40
    //schedule[0].day_of_week = 1;
    //schedule[0].start_time = (11*60 + 40);
    
    schedule[0].enabled = 1;
    schedule[0].day_of_week = EVERYDAY;
    schedule[0].start_time = (5*60 + 2);
    schedule[0].duration = 2;
 
    now = time(nullptr);
    Serial.printf("curr time: %d\n\r", (unsigned int)now);

    for (int i=0; i<MAX_ENTRIES; i++) {
        if (schedule[i].enabled) {
            on_events[i] = next_occurrence_start(&schedule[i], now);
            off_events[i] = next_occurrence_end(&schedule[i], now);
            events_nr++;
        }
    }
        
    //printf("\n\r");
    //next = next_occurrence(&schedule[0], now + (i * SECS_PER_WEEK));
    //Serial.printf("next: %d\n\r", (unsigned int)next);
    //
    //localtime_r(&next, &tm_now);
    //printTm("occurence", &tm_now);
    
    Serial.printf("events_nr: %d\n\r", events_nr);
}


void next_event(time_t * time, uint8_t * is_on) {

    *time = UINT_MAX;

    for(int i=0; i<events_nr; i++) {
        
        if (on_events[i] < *time) {
            *time = on_events[i];
            *is_on = 1;

        }
        if (off_events[i] < *time) {
            *time = off_events[i];
            *is_on = 0;
        }
    }
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

    delay(2000);

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
    parse_schedule();

    Serial.println("end setup\n");
}


void loop() {

    //Serial.println("LOOP");
    //open_valve();
    //delay(5000);
    //close_valve();
    //delay(5000);

    now = time(nullptr);
    Serial.print("UTC time from system:  ");
    Serial.println((unsigned int)now);

    Serial.print("UTC time from rtc:     ");
    now = rtc_get_time();
    Serial.println((unsigned int)now);

    localtime_r(&now, &tm_now);
    printTm("localtime from system:", &tm_now);

    time_t next;
    uint8_t is_on;
    struct tm next_tm;

    Serial.println("");
    next_event(&next, &is_on);
    Serial.printf("next: %d\n\r", (unsigned int)next);

    localtime_r(&next, &next_tm);
    printTm("next_event (localtime):", &next_tm);
    Serial.printf("is_on: %d\n\r", is_on);
    
    
    uint8_t next_op, curr_op;
    unsigned int sleeptime;
    
    rtc.readRam(ADDR_NEXTOP, &curr_op);
    
    switch (curr_op) {
        
        case OP_SKIP:
            break;

        case OP_OPEN:
            open_valve();
            break;

        case OP_CLOSE:
            close_valve();
            break;

        default:
            break;

    }

    if (is_on) {
        next_op = OP_OPEN;
    }
    else {
        next_op = OP_CLOSE;
    }

    sleeptime = (unsigned int)next-now; 
    
    if (sleeptime > SLEEP_MAX) {
        sleeptime = SLEEP_MAX;
        next_op = OP_SKIP;
    }
        
    rtc.writeRam(ADDR_NEXTOP, next_op);
    
    Serial.printf("Will sleep for: %d\n\r", sleeptime);
    sleeptime = sleeptime * 1e6;

    ESP.deepSleep(sleeptime);
}


