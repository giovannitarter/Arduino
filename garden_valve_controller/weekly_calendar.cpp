#include <ESP8266WiFi.h>
#include <time.h>
#include <cstdio>
#include "LittleFS.h"

#include "pb_encode.h"
#include "pb_decode.h"

#include "weekly_calendar.h"
#include "schedule.pb.h"

#include "garden_valve_controller.h"


bool Schedule_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_iter_t * field) {

    ScheduleEntry ent;
    WeeklyCalendar * wkp;

    if (ostream && !istream) {
        PB_UNUSED(istream);

        wkp = (WeeklyCalendar *) field->pData;

        if (field->tag == Schedule_events_tag) {
            ent = {
                true,
                true,
                true,
                WeekDay_EVR,
                true,
                Operation_OP_OPEN,
                true,
                7,
                true,
                0
            };
            
            if (!pb_encode_tag_for_field(ostream, field))
                return false;
            
            if (!pb_encode_submessage(ostream, ScheduleEntry_fields, &ent))
                return false;
            
            ent = {
                true,
                true,
                true,
                WeekDay_EVR,
                true,
                Operation_OP_CLOSE,
                true,
                7,
                true,
                2
            };
            
            if (!pb_encode_tag_for_field(ostream, field))
                return false;
            
            if (!pb_encode_submessage(ostream, ScheduleEntry_fields, &ent))
                return false;

        }
    }
    else if(istream && !ostream) {

        PB_UNUSED(ostream);
        
        wkp = (WeeklyCalendar *) field->pData;
        
        if (field->tag == Schedule_events_tag) {
            
            if (!pb_decode(istream, ScheduleEntry_fields, &ent))
                return false;

            wkp->add_event(&ent);
            Serial.printf("evt at %d:%d op: %d\n\r", ent.start_hou, ent.start_min, ent.op);
        }
    }

    return true;
}


WeeklyCalendar::WeeklyCalendar() {

}




uint8_t WeeklyCalendar::add_event(ScheduleEntry * ent) {
    
    time_t time, period, offset, tmp;
    
    time = ent->start_hou * SECS_PER_HOU + ent->start_min * SECS_PER_MIN;

    Serial.printf("Add event: %d\n\r", time);

    period = _get_period(ent->wday);
    offset = _get_offset(ent->wday, time);
    
    //last event
    tmp = _last_occurrence(offset, _ctime, period);

    //next event
    tmp += period;

    if (tmp < _next) {
        _next = tmp;
        _next_op = ent->op;
    }

    return 0;   
};


uint8_t WeeklyCalendar::next_event_r(
        time_t ctime, 
        time_t last_executed, 
        uint8_t * op,
        time_t * sleeptime
        )
{

    uint8_t buffer[128];
    time_t tmp, next, period, offset;
    size_t len;
    
    len = 0;

    LittleFS.begin();
    if (LittleFS.exists("/schedule.txt")) {
        
        File f = LittleFS.open("schedule.txt", "r");

        len = f.size();
        f.readBytes((char *)buffer, len);

        f.close();
    }
    LittleFS.end();

    Serial.printf("read %d bytes\n\r", len);

    //performing all actions between (time + EVT_TOLERANCE) and (ctime + EVT_TOLERANCE)
    *op = OP_SKIP;
    
    _ctime = ctime - EVT_TOLERANCE;
    //_next = ctime + EVT_TOLERANCE;
    _next = UINT_MAX;
    _next_op = OP_SKIP;
    
    Serial.printf("ref time: %d\n\r", (unsigned int)ctime);
    
    Schedule msg; 
    pb_istream_t istream;
    uint8_t status;

    istream = pb_istream_from_buffer(buffer, len);
    msg.events = (void *) this;
    status = pb_decode(&istream, Schedule_fields, &msg);

    print_time_t("next_op: ", _next, 0);

    //now = time(nullptr);
    //sleeptime = (unsigned int)next - now; 
    //
    ////sleep at most SLEEP_MAX seconds 
    //if (sleeptime > SLEEP_MAX) {
    //    sleeptime = SLEEP_MAX;
    //}
    //else if (sleeptime > VALVE_DELAY) {
    //    
    //    //next time the micro wakes up, there will be an action to perform,
    //    //adjust for VALVE_DELAY
    //    sleeptime -= VALVE_DELAY;
    //}
    //else if (sleeptime < EVT_TOLERANCE) {
    //    sleeptime = EVT_TOLERANCE;
    //}
    
    return 0;
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
