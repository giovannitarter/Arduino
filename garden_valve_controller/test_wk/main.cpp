#include <stdio.h>
#include <stdlib.h>
#include <pb_encode.h>

#include "weekly_calendar.h"
#include "schedule.pb.h"
#include "main.h"


bool enc_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void * const *arg) {

    ScheduleEntry ent;

    if (field->tag == Schedule_events_tag) {
        ent = {
            true,
            true,
            true,
            WeekDay_EVR,
            true,
            Operation_OP_OPEN,
            true,
            16,
            true,
            33
        };

        if (!pb_encode_tag_for_field(stream, field))
            return false;

        if (!pb_encode_submessage(stream, ScheduleEntry_fields, &ent))
            return false;

        ent = {
            true,
            true,
            true,
            WeekDay_EVR,
            true,
            Operation_OP_CLOSE,
            true,
            16,
            true,
            34
        };

        if (!pb_encode_tag_for_field(stream, field))
            return false;

        if (!pb_encode_submessage(stream, ScheduleEntry_fields, &ent))
            return false;

        ent = {
            true,
            true,
            true,
            WeekDay_EVR,
            true,
            Operation_OP_OPEN,
            true,
            16,
            true,
            40
        };

        if (!pb_encode_tag_for_field(stream, field))
            return false;

        if (!pb_encode_submessage(stream, ScheduleEntry_fields, &ent))
            return false;

        ent = {
            true,
            true,
            true,
            WeekDay_EVR,
            true,
            Operation_OP_OPEN,
            true,
            16,
            true,
            45
        };

        if (!pb_encode_tag_for_field(stream, field))
            return false;

        if (!pb_encode_submessage(stream, ScheduleEntry_fields, &ent))
            return false;

    }

    return true;
}


void write_schedule(uint8_t * buffer, size_t * len) {

    bool status;
    Schedule msg;

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, *len);
    msg.version = 1;
    msg.events.funcs.encode = &enc_callback;
    msg.events.arg = nullptr;

    status = pb_encode(&stream, Schedule_fields, &msg);
    *len = stream.bytes_written;
}




int main(int argc, char * argv[]) {

    time_t now;

    //setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    //now = 1648317600;
    //now = 1667066400;

    //setenv("TZ", "GMTGMT-1,M3.4.0/01,M10.4.0/02", 1);

    //setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
    //now = 1667671200;

    WeeklyCalendar wk;
    uint8_t buffer[256];
    size_t len;

    uint32_t sleeptime;
    uint8_t op;

    len = 256;
    write_schedule(buffer, &len);
    printf("len: %d\n", len);

    //current time
    //now = time(nullptr);

    //setting now to 2022-10-29 18.00.00
    //to test daylight save time change

    //struct tm lt = {0};
    //localtime_r(&now, &lt);
    //printf("Offset to GMT is %lds.\n", lt.tm_gmtoff);
    //printf("The time zone is '%s'.\n", lt.tm_zone);

    for(int i=0; i<5; i++) {

        printf("\n========================\n");
        sleeptime = 0;
        op = Operation_OP_NONE;
        wk.init(now, buffer, len, 10);
        while(sleeptime == 0) {
            wk.next_event(now, &op, &sleeptime);
            printf("sleeptime: %d\n", sleeptime);
        }

        now += sleeptime;
    }

    return 0;

}
