#include <ESP8266WiFi.h>
#include <time.h>
#include <cstdio>
#include "LittleFS.h"

#include "pb_encode.h"
#include "pb_decode.h"

#include "weekly_calendar.h"
#include "schedule.pb.h"

#include "garden_valve_controller.h"


bool dec_callback(pb_istream_t *istream, const pb_field_iter_t *field, void **arg) {

    WeeklyCalendar * wk = (WeeklyCalendar *) *arg;
    ScheduleEntry ent;

    if (field->tag == Schedule_events_tag) {

        if (!pb_decode(istream, ScheduleEntry_fields, &ent))
            return false;

        wk->add_event(&ent);
    }
    return true;
}


WeeklyCalendar::WeeklyCalendar() {
}


uint8_t WeeklyCalendar::add_event(ScheduleEntry * ent) {

    time_t time, period, offset, tmp;

    time = ent->start_hou * SECS_PER_HOU + ent->start_min * SECS_PER_MIN;

    period = _get_period(ent->wday);
    offset = _get_offset(ent->wday, time);

    //last event
    tmp = _last_occurrence(offset, _ctime, period);

    //next event
    tmp += period;

    //print_time_t("next_occurrence: ", tmp, 1);
    //
    uint8_t action;

    switch (ent->op) {
        
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

    }

    _events[_ev_next].time = tmp;
    _events[_ev_next].action = action;
    _ev_next = (_ev_next + 1) % MAX_EVENTS;
    _ev_nr++;

    return 0;
};


int compar(const void * a, const void * b) {
    return (((event_t *)a)->time - ((event_t *)b)->time);
}


uint8_t WeeklyCalendar::init(time_t ctime) {
    
    uint8_t buffer[128];
    size_t len;
    uint8_t res = 0;

    _ctime = ctime - EVT_TOLERANCE;
    _next_exec = 0;

    len = 0;
    LittleFS.begin();
    if (LittleFS.exists("/schedule.txt")) {

        File f = LittleFS.open("/schedule.txt", "r");

        len = f.size();
        f.readBytes((char *)buffer, len);

        f.close();
    }
    LittleFS.end();

    Serial.printf("read %d bytes\n\r", len);

    _ev_next = 0;
    for(int i=0; i<MAX_EVENTS; i++) {
        _events[i].time = UINT_MAX;
        _events[i].action = OP_NONE;
    }

    Serial.printf("ref time: %d\n\r", (unsigned int)ctime);

    Schedule msg;
    pb_istream_t istream;
    uint8_t status;

    msg.events.funcs.decode = &dec_callback;
    msg.events.arg = (void *) this;
    istream = pb_istream_from_buffer(buffer, len);
    status = pb_decode(&istream, Schedule_fields, &msg);

    qsort(_events, _ev_nr, sizeof(event_t), &compar);

    for(int i=0; i<_ev_nr; i++) {
        Serial.printf("%d -> action: %X ", i, _events[i].action);
        print_time_t("time: ", _events[i].time, 0);
    }
    return 0;
}


uint8_t WeeklyCalendar::next_event(
        uint8_t * op,
        uint32_t * sleeptime
        )
{

    uint8_t res = 0;
    time_t next, now;
    
    *op = OP_NONE;

    next = _events[_next_exec].time;
    if (next < _ctime + 2 * EVT_TOLERANCE) 
    {
        *op = _events[_next_exec].action;
        *sleeptime = 0;
        _next_exec++;
        
        Serial.printf("Performing action %d ", *op);
        print_time_t("scheduled at: ", _events[_next_exec].time, 0);
    }
    else {
        now = time(nullptr);
        *sleeptime = (unsigned int)next - (unsigned int)now;

        Serial.println(next);
        Serial.println(now);
        Serial.println((unsigned int)*sleeptime);

        if (*sleeptime > BOOT_DELAY) 
            *sleeptime -= BOOT_DELAY;
    
        //if (*sleeptime < EVT_TOLERANCE) {
        //    Serial.printf("sleeptime set to %d, fixed to %d\n\r", *sleeptime, EVT_TOLERANCE);
        //    *sleeptime = EVT_TOLERANCE;
        //}
        
        if (*sleeptime > SLEEP_MAX) {
            *sleeptime = SLEEP_MAX;
        }
    }
    

    return res;
}


time_t WeeklyCalendar::_last_occurrence(time_t offset, time_t time, time_t period) {

    time_t res;

    //first we compute the occurence since thursday 00:00
    res = offset;

    //last_occurrence
    res = time - ((time - res) % period);

    return res;
}


time_t WeeklyCalendar::_get_offset(uint8_t wday, time_t time) {

    //start_time in secs

    //wse is considered as happening in the first week after epoch
    //Returns offset in seconds from epoch (which is a Thusrday)
    //i.e.:
    //Monday: (THR + FRI + SAT + SUN) * SECONDS_PER_DAY

    time_t offset;
    offset = 0;

    if (wday != WeekDay_EVR) {
        offset += (wday + THU_TO_SUN) * SECS_PER_DAY;
    }

    offset += time;

    return offset;
}


time_t WeeklyCalendar::_get_period(uint8_t wday) {

    time_t period;

    if (wday != WeekDay_EVR) {
        period = SECS_PER_WEEK;
    }
    else {
        period = SECS_PER_DAY;
    }

    return period;
}


void WeeklyCalendar::print_time_tm(char * text, struct tm * prt_time) {

    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", prt_time);
    Serial.printf("%s%s\n\r", text, buffer);
}


void WeeklyCalendar::print_time_t(char * text, time_t t, uint8_t utc) {

    char buffer[26];
    struct tm tmp;

    if (utc) {
        gmtime_r(&t, &tmp);
    }
    else {
        localtime_r(&t, &tmp);
    }

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", &tmp);
    Serial.printf("%s%s\n\r", text, buffer);
}
