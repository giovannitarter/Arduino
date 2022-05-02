#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <sntp.h>
#include <ctime>
#include <HTTPClient.h>

#include "garden_valve_controller.h"
#include "Ds1302.h"
#include "weekly_calendar.h"
#include "solenoid_driver.h"

#define PIN_CLK 21
#define PIN_DIO 22
#define PIN_CE  2

#define PIN_SU   16
#define PIN_STBY 4 

#define PIN_H_A1 21
#define PIN_H_A2 22

#define PIN_LED_CONFIG 23
#define PIN_CONFIG 34

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

#define SCHEDULE_MAXLEN 256
uint8_t buffer[SCHEDULE_MAXLEN];
size_t buflen;


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
    struct tm now_tm;

    //settimeofday_cb(time_is_set);

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

        configTime(0, 0, "pool.ntp.org");
        getLocalTime(&now_tm);
        wk.print_time_tm("localtime: ", &now_tm);
        time_is_set(true);

        WiFiClient client;
        HTTPClient http;
        http.begin(client, "http://192.168.2.25:25000/getschedule");
        
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            //Serial.println(http.getSize());
          
            LittleFS.begin(true);
            File f = LittleFS.open("/schedule.txt", "w");
            uint8_t buff[20] = { 0 };
            int len = http.getSize();

            while (http.connected() && (len > 0 || len == -1)) {
                // read up to 128 byte
                int c = client.readBytes(buff, std::min((size_t)len, sizeof(buff)));

                // write it to Serial
                f.write(buff, c);

                if (len > 0) { 
                    len -= c; 
                }
            }
            f.close();
            LittleFS.end();
        }
        http.end();
    }
    else {
        Serial.println("RTC is ok, setting time to system clock");
        system_set_time(rtc_time);
    }
}


uint8_t decode_action(uint8_t msg_action) {

    uint8_t action;

    switch (msg_action) {
        
        case Operation_OP_OPEN:
           action = OP_OPEN;
           break;
        
        case Operation_OP_CLOSE:
           action = OP_CLOSE;
           break;
        
        case Operation_OP_SKIP:
           action = OP_SKIP;
           break;

        default:
           action = OP_NONE;
            break;
    }

    return action;
}


size_t read_schedule(char * buffer, size_t buflen) {

    size_t res = 0;

    Serial.println("read schedule");

    LittleFS.begin(true);
    if (LittleFS.exists("/schedule.txt")) {

        File f = LittleFS.open("/schedule.txt", "r");
        res = f.readBytes(buffer, buflen);
        f.close();
    }
    LittleFS.end();

    return res;
}


void setup() {

    pinMode(PIN_SU, OUTPUT);
    pinMode(PIN_STBY, OUTPUT);
    pinMode(PIN_LED_CONFIG, OUTPUT);
    pinMode(PIN_CONFIG, INPUT);

    digitalWrite(PIN_SU, LOW);
    digitalWrite(PIN_STBY, LOW);

    rtc.init();
    soldrv.init(PIN_SU, PIN_STBY, PIN_H_A1, PIN_H_A2);

    int config;

    delay(BOOT_DELAY * 9e2);
    
    digitalWrite(PIN_LED_CONFIG, HIGH);
    delay(BOOT_DELAY * 1e2);
    digitalWrite(PIN_LED_CONFIG, LOW);
    
    config = digitalRead(PIN_CONFIG);

    Serial.begin(115200);
    Serial.println("\n\nBOOT");

    Serial.printf("config: %d\n\r", config);
    Serial.printf("STASSID: %s\n\r", STASSID);
    Serial.printf("STAPSK: %s\n\r", STAPSK);

    dir = 0;

    set_time_boot(config);
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

    uint32_t sleeptime;
    uint8_t op;

    Serial.println("");

    sleeptime = 0;
    op = OP_NONE;
    buflen = SCHEDULE_MAXLEN;
    
    buflen = read_schedule((char *)buffer, buflen);

    //while(1) {
    //    
    //    Serial.println("Time to open");
    //    soldrv.open_valve();
    //    delay(5000);
    //    
    //    Serial.println("Time to close");
    //    soldrv.close_valve();
    //    delay(5000);
 
    //}
    //return;


    wk.init(now, buffer, buflen, EVT_TOLERANCE);

    while(sleeptime == 0) {

        wk.next_event(now, &op, &sleeptime);
        
        Serial.printf("op1: %X\n\r", op);
        op = decode_action(op);
        Serial.printf("op2: %X\n\r", op);

        switch (op) {

            case OP_SKIP:
                Serial.println("Time to skip");
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
    }
    
    time_t req_time = time(nullptr) - now;
    if (req_time < 0) {
        req_time = 0;
    }
    
    Serial.printf("All action performed in %d sec\n\r", (uint32_t) req_time);

    if (sleeptime > SLEEP_MAX) {
        sleeptime = SLEEP_MAX;
    }
    else {
        sleeptime -= req_time;
        sleeptime -= BOOT_DELAY;
    }

    Serial.printf("Will sleep for: %d\n\r", sleeptime);
    sleeptime = sleeptime * 1e6;
    ESP.deepSleep(sleeptime);
}

