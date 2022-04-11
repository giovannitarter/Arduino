#include <ESP8266WiFi.h>
#include <coredecls.h>
#include <sntp.h>
#include <Wire.h>


const int8_t I2C_SLAVE = 0x68;

#define PIN_SDA 5 //D1
#define PIN_SCL 4 //D2

#define PIN_SU   14 //D5
#define PIN_STBY 12 //D6
#define PIN_H_A1 13 //D7
#define PIN_H_A2 2  //D4


#define TIME_MIN 1628141636

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif


const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";


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


void time_is_set(bool from_sntp) {
  
  Serial.print("time is set - ");
  Serial.println(from_sntp);
  
  if (from_sntp) {
    Serial.println("Writing time to rtc");
    
    time_t now = time(nullptr);
    rtc_set_time(now);

  }
}


uint8_t rtc_read_at_address(uint8_t dev_addr, uint8_t mem_addr, uint8_t size, uint8_t * data) {

  uint8_t i;
  
  Wire.beginTransmission(dev_addr);
  Wire.write(mem_addr);
  Wire.endTransmission();
  
  Wire.requestFrom(dev_addr, size);
  i = 0;
  while(Wire.available() && i < size) {
    data[i] = Wire.read();
    i++;
  }

  return(i);
}


uint8_t rtc_write_at_address(uint8_t dev_addr, uint8_t mem_addr, uint8_t size, uint8_t * data) {

  uint8_t i;
  
  Wire.beginTransmission(dev_addr);
  Wire.write(mem_addr);
  
  i = 0;
  while (i < size) {
    Wire.write(data[i]);
    i++;
  }
  Wire.endTransmission();
  return(i);
}



uint8_t rtc_failed() {

  uint8_t status, res;
  res = rtc_read_at_address(I2C_SLAVE, 0x0F, 1, &status);
  return (status & 0b10000000);
}

void dump_data(uint8_t * data, uint8_t size) {

    Serial.print("data:");
    for(int i=0; i<size; i++) {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");

}


void rtc_set_time(time_t time_p) {

  struct tm * tm;
  uint8_t data[7];

  tm = localtime(&time_p);
  data[0] = tm->tm_sec / 10;
  data[0] <<= 4;
  data[0] += tm->tm_sec % 10;

  data[1] = tm->tm_min / 10;
  data[1] <<= 4;
  data[1] += tm->tm_min % 10;

  data[2] = tm->tm_hour / 10;
  data[2] <<= 4;
  data[2] += tm->tm_hour % 10;
  
  data[4] = (tm->tm_mday / 10) << 4;
  data[4] += tm->tm_mday % 10;

  data[5] = (tm->tm_mon / 10) << 4;
  data[5] += tm->tm_mon % 10;
  data[5] += 1;

  data[6] = (tm->tm_year / 10) << 4;
  data[6] += tm->tm_year % 10;

  rtc_write_at_address(I2C_SLAVE, 0x00, 7, data);

  data[0] = 0;
  rtc_write_at_address(I2C_SLAVE, 0x0F, 1, data);

  free(tm);
}


void system_set_time(time_t curr_time) {
    timeval tv = {curr_time, 0};
    settimeofday(&tv, nullptr);
    return;
}



time_t rtc_get_time() {

  uint8_t data[7], read_bytes;
  time_t res = 0;
  
  read_bytes = rtc_read_at_address(I2C_SLAVE, 0, 7, data);
  if (read_bytes) {

    struct tm tmp;
    
    tmp.tm_sec = data[0] & 0x0F;
    data[0] >>= 4;
    tmp.tm_sec += data[0] * 10;

    tmp.tm_min = data[1] & 0x0F;
    data[1] >>= 4;
    tmp.tm_min += data[1] * 10;

    tmp.tm_hour = data[2] & 0x0F;
    data[2] >>= 4;
    tmp.tm_hour += data[2] * 10;

    tmp.tm_mday = data[4] & 0x0F;
    data[4] >>= 4;
    tmp.tm_mday += data[4] * 10;

    tmp.tm_mon = data[5] & 0x0F;
    data[5] >>= 4;
    tmp.tm_mon += (data[5] & 0b0001) * 10;
    tmp.tm_mon -= 1;

    tmp.tm_year = data[6] & 0x0F;
    data[6] >>= 4;
    tmp.tm_year += data[6] * 10;

    //printTm("read time:", &tmp);
    //printTm("read_time: ", localtime(&read_time));

    res = mktime(&tmp);
  }
  else {
    Serial.println("Error reading from RTC");
  }
  return(res);
}



time_t now;
struct tm tm;
int dir;

void setup() {

  uint8_t need_sync, rtc_status;
  time_t rtc_time;

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

  Wire.begin(PIN_SDA, PIN_SCL);
  
  settimeofday_cb(time_is_set);
  configTime(TZ_INFO, "pool.ntp.org");

  rtc_time = rtc_get_time();
  Serial.print("rtc_time: ");
  Serial.println(rtc_time);

  rtc_status = rtc_failed();
  Serial.print("rtc_status: ");
  Serial.println(rtc_status);
  
  need_sync = (rtc_time < TIME_MIN || rtc_status);

  Serial.print("need_sync: ");
  Serial.println(need_sync);

  if (need_sync) {

    Serial.println("Error on RTC, sync from the internet");
        
    now = time(nullptr);
    printTm("localtime:", localtime(&now));
    Serial.println();
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(STASSID, STAPSK);

    uint32_t timeout = millis() + 5000;
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


void loop() {


    Serial.println("");

    now = rtc_get_time();
    Serial.print("rtc_time: ");
    Serial.println(now);


    now = time(nullptr);
    Serial.print("localtime: ");
    Serial.println(now);

    localtime_r(&now, &tm);
    printTm("localtime:", &tm);

    delay(10000);

}
