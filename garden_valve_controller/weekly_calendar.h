#ifndef __GVC_WEEKLY_CALENDAR
#define __GVC_WEEKLY_CALENDAR


#define STRUCT_VERSION 0x01
#define EVERYDAY 7
#define MAX_ENTRIES 10
#define SECS_PER_WEEK 604800
#define SECS_PER_DAY 86400
#define SECS_PER_MIN 60
#define THU_TO_SUN 3


//weekly schedule descriptr
struct ws_entry {
    
    uint8_t version;
    uint8_t enabled;

    //sunday = 0
    uint8_t day_of_week;

    //in minutes since midnight
    uint16_t start_time;

    //in minutes
    uint16_t duration;
};
typedef struct ws_entry ws_entry;


class WeeklyCalendar {
        
    public:
        WeeklyCalendar(uint8_t entries);
        void parse_schedule();
        
        uint8_t next_event(
                time_t ctime, 
                time_t *last, time_t *next, 
                uint8_t *last_op, uint8_t *next_op
                );
        
        void print_time_tm(char * text, struct tm * prt_time);
        void print_time_t(char * text, time_t t, uint8_t utc);

    private:
        
        ws_entry schedule[MAX_ENTRIES];
        
        int events_nr;
        
        time_t _last_occurrence(time_t offset, time_t time, time_t period);
        time_t _get_period(ws_entry * wse);
        time_t _get_offset(ws_entry * wse);
        
};



#endif
