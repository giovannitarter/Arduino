#include <ESP8266WiFi.h>
#include <time.h>
#include <cstdio>

#include "pb_encode.h"
#include "pb_decode.h"
#include "schedule.pb.h"

#include "garden_valve_controller.h"
#include "weekly_calendar.h"


bool Schedule_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_iter_t * field) {

    ScheduleEntry ent;

    if (ostream && !istream) {
        PB_UNUSED(istream);

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
        if (field->tag == Schedule_events_tag) {
            
            if (!pb_decode(istream, ScheduleEntry_fields, &ent))
                return false;

            Serial.printf("evt at %d:%d op: %d\n\r", ent.start_hou, ent.start_min, ent.op);
        }
    }

    return true;
}


WeeklyCalendar::WeeklyCalendar() {

}


void WeeklyCalendar::parse_schedule() {
    
    bool status;
    size_t len;
    Schedule msg;
    // = Schedule_init_zero;

    uint8_t buffer[128];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    msg.version = 1;

    status = pb_encode(&stream, Schedule_fields, &msg);
    len = stream.bytes_written;

    pb_istream_t istream;
    istream = pb_istream_from_buffer(buffer, len);
    status = pb_decode(&istream, Schedule_fields, &msg);
}


uint8_t WeeklyCalendar::next_event_r(
        time_t ctime, 
        time_t last_executed, 
        uint8_t * op,
        time_t * sleeptime
        )
{

    time_t tmp, next, period, offset;

    //performing all actions between (time + EVT_TOLERANCE) and (ctime + EVT_TOLERANCE)
    *op = OP_SKIP;
    next = ctime + EVT_TOLERANCE;
    ctime = ctime - EVT_TOLERANCE;
    
    Serial.printf("ref time: %d\n\r", (unsigned int)ctime);

    //for (int i=0; i<events_nr; i++) {
    //    
    //    Serial.println("");
    //    
    //    if (schedule[i].enabled) {

    //        period = _get_period(&schedule[i]);
    //        offset = _get_offset(&schedule[i]);
    //        
    //        //last event
    //        tmp = _last_occurrence(offset, ctime, period);

    //        //next event
    //        tmp += period;
    //        
    //        if (tmp < *next) {
    //            *next = tmp;
    //            *op = schedule[i].op;
    //        }
    //    }
    //}
    
    
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
