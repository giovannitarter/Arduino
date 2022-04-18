#ifndef __GVC_WEEKLY_CALENDAR
#define __GVC_WEEKLY_CALENDAR


#define STRUCT_VERSION 0x01
#define EVERYDAY 7
#define MAX_ENTRIES 10
#define SECS_PER_WEEK 604800
#define SECS_PER_DAY 86400
#define SECS_PER_MIN 60
#define THU_TO_SUN 3


class WeeklyCalendar {
        
    public:
        WeeklyCalendar();
        void parse_schedule();
        
        uint8_t next_event_r(
                time_t ctime, 
                time_t last_executed, 
                uint8_t *op,
                time_t * sleeptime
                );
        
        void print_time_tm(char * text, struct tm * prt_time);
        void print_time_t(char * text, time_t t, uint8_t utc);

    private:
        
        time_t _last_occurrence(time_t offset, time_t time, time_t period);
        time_t _get_period(uint8_t wday);
        time_t _get_offset(uint8_t wday, time_t time);
};



#endif
