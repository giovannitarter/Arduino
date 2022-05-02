#include <stdio.h>
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

    WeeklyCalendar wk;
    uint8_t buffer[256];
    size_t len;
    time_t now;

    uint32_t sleeptime;
    uint8_t op;

    len = 256;
    write_schedule(buffer, &len);
    printf("len: %d\n", len);
    
    now = time(nullptr);

    for(int i; i<5; i++) {
        
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
