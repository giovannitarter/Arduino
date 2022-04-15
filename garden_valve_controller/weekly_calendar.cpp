#include <ESP8266WiFi.h>
#include <time.h>
#include <cstdio>

#include "garden_valve_controller.h"
#include "weekly_calendar.h"


WeeklyCalendar::WeeklyCalendar(uint8_t entries) {

}


void WeeklyCalendar::parse_schedule() {
    
    events_nr = 0;

    memset(schedule, 0, sizeof(ws_entry) * MAX_ENTRIES);
    
    ////next monday at 11.40
    //schedule[0].enabled = 1;
    //schedule[0].day_of_week = 1;
    //schedule[0].start_time = (11*60 + 40);
    //schedule[0].duration = 15;
    //events_nr++;
    
    schedule[0].enabled = 1;
    schedule[0].day_of_week = EVERYDAY;
    schedule[0].start_time = (14*60 + 15);
    schedule[0].duration = 1;
    events_nr++;
  
    Serial.printf("events_nr: %d\n\r", events_nr);
}


uint8_t WeeklyCalendar::next_event(
        time_t ctime, 
        time_t * last, 
        time_t * next, 
        uint8_t * last_op,
        uint8_t * next_op
        ) 
    
{

    time_t tmp, evt_time, period, offset;

    *next = UINT_MAX;
    *last = 0;
    
    *last_op = OP_SKIP;
    *next_op = OP_SKIP; 
    
    Serial.printf("ref time: %d\n\r", (unsigned int)ctime);

    for (int i=0; i<events_nr; i++) {
        
        Serial.println("");
        
        if (schedule[i].enabled) {

            period = _get_period(&schedule[i]);
            offset = _get_offset(&schedule[i]);
            
            //last on event
            tmp = _last_occurrence(offset, ctime, period);
            if (tmp > *last) {
                *last = tmp;
                *last_op = OP_OPEN;
            }
            print_time_t("last_on:  ", tmp, 0);

            //next on event
            tmp += period;
            if (tmp < *next) {
                *next = tmp;
                *next_op = OP_OPEN;
            }
            print_time_t("next_on:  ", tmp, 0);

            offset += schedule[i].duration * SECS_PER_MIN;
            
            //last off event
            tmp = _last_occurrence(offset, ctime, period);
            if (tmp > *last) {
                *last = tmp;
                *last_op = OP_CLOSE;
            }
            print_time_t("last_off: ", tmp, 0);

            //next off event
            tmp += period;
            if (tmp < *next) {
                *next = tmp;
                *next_op = OP_CLOSE;
            }
            print_time_t("next_off: ", tmp, 0);
        }
    }
    
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


time_t WeeklyCalendar::_get_offset(ws_entry * wse) {

    //wse is considered as happening in the first week after epoch
    //Returns offset in seconds from epoch (which is a Thusrday)
    //i.e.:
    //Monday: (THR + FRI + SAT + SUN) * SECONDS_PER_DAY

    time_t offset;
    offset = 0;
    
    if (wse->day_of_week != EVERYDAY) {
        offset += (wse->day_of_week + THU_TO_SUN) * SECS_PER_DAY;
    }
    
    offset += wse->start_time * SECS_PER_MIN;
    
    return offset;
}


time_t WeeklyCalendar::_get_period(ws_entry * wse) {

    time_t period;
    
    if (wse->day_of_week != EVERYDAY) {
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
