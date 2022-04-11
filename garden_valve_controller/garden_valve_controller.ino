#include <ESP8266WiFi.h>
#include <coredecls.h>
#include <sntp.h>
#include "Ds1302.h"


#define PIN_CLK 5 //D1
#define PIN_DIO 4 //D2
#define PIN_CE  10 //SV2

#define PIN_SU   14 //D5
#define PIN_STBY 12 //D6
#define PIN_H_A1 13 //D7
#define PIN_H_A2 2  //D4


#define TIME_MIN 1649751185

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

//Globals
int dir;
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

Ds1302 rtc = Ds1302(PIN_CE, PIN_CLK, PIN_DIO);


typedef struct saved_time {
  uint32_t time;
  uint32_t enabled;
} saved_time;

saved_time st;

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

  Serial.print("time is set - ");
  Serial.println(from_sntp);

  if (from_sntp) {
    Serial.println("Writing time to rtc");

    time_t now = time(nullptr);
    rtc_set_time(now);

  }
}


void system_set_time(time_t curr_time) {
    timeval tv = {curr_time, 0};
    settimeofday(&tv, nullptr);
    return;
}


void rtc_set_time(time_t time_p) {

    struct tm * tmp;

    tmp = localtime(&time_p);
    rtc.setDateTime(tmp);

    free(tmp);
}


time_t rtc_get_time() {

    time_t res = 0;
    struct tm tmp;

    rtc.getDateTime(&tmp);
    res = mktime(&tmp);

    return(res);
}


void set_time_boot() {

    uint8_t need_sync;
    time_t rtc_time, now;

    settimeofday_cb(time_is_set);
    configTime(TZ_INFO, "pool.ntp.org");

    rtc_time = rtc_get_time();
    Serial.print("rtc_time: ");
    Serial.println((unsigned int)rtc_time);

    //rtc_status = rtc_failed();
    //Serial.print("rtc_status: ");
    //Serial.println(rtc_status);

    need_sync = rtc_time < TIME_MIN;

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


void setup() {

    delay(2000);
    Serial.begin(115200);
    Serial.println("\n\nBOOT");

    pinMode(PIN_SU, OUTPUT);
    pinMode(PIN_STBY, OUTPUT);
    pinMode(PIN_H_A1, OUTPUT);
    pinMode(PIN_H_A2, OUTPUT);

    digitalWrite(PIN_SU, LOW);
    digitalWrite(PIN_STBY, LOW);
    digitalWrite(PIN_H_A1, LOW);
    digitalWrite(PIN_H_A2, LOW);

    dir = 0;

    rtc.init();

    Serial.println("CANE");

    set_time_boot();
}


// the loop function runs over and over again forever
void loop() {

    //ESP.deepSleep(10e6);

    time_t now = time(nullptr);
    Serial.print("time:");
    Serial.println((unsigned int)now);

    delay(10000);
    return;

  Serial.println("");
  Serial.print("LOOP -: dir: ");
  Serial.println(dir);

  if (dir) {
    dir = 0;
  }
  else {
    dir = 1;
  }

  digitalWrite(PIN_SU, HIGH);

  Serial.println("Charging...");
  delay(5000);
  Serial.println("Charged!");

  if (dir) {
    digitalWrite(PIN_H_A1, HIGH);
    digitalWrite(PIN_H_A2, LOW);
  }
  else {
    digitalWrite(PIN_H_A1, LOW);
    digitalWrite(PIN_H_A2, HIGH);
  }

  Serial.println("Activation");
  digitalWrite(PIN_STBY, HIGH);
  delay(75);
  Serial.println("Stop");

  digitalWrite(PIN_STBY, LOW);
  digitalWrite(PIN_SU, LOW);
  digitalWrite(PIN_H_A1, LOW);
  digitalWrite(PIN_H_A2, LOW);

  Serial.println("Waiting");
  delay(10000);

}
