#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>


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


void print_time(char * text, struct tm * prt_time) {

    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", prt_time);
    printf("%s%s\n", text, buffer);
}


time_t get_timezone_offset(time_t time) {


    struct tm ptm;
    time_t gmt, offset;
    gmtime_r(&time, &ptm);
    
    // Request that mktime() looksup dst in timezone database
    ptm.tm_isdst = -1;                
    gmt = mktime(&ptm);
    
    offset = time - gmt;
    printf("%d\n", offset);
    return offset;
}


//computes next occurrence (returned as epoch time) 
//after time given the schedule entry
time_t next_occurrence(ws_entry * wse, time_t time) {

    time_t res, offset;
    struct tm local_time;

    res = 0;

    //first we compute the occurence since thursday 00:00
    if (wse->day_of_week != EVERYDAY) {
        res += (wse->day_of_week + THU_TO_SUN) * SECS_PER_DAY;
    }
    
    res += wse->start_time * SECS_PER_MIN;
    
    //last_occurrence
    res = time - ((time - res) % SECS_PER_WEEK);
    
    //next_occurrence
    res += SECS_PER_WEEK;

    //Timezone and dts adjustment
    offset = get_timezone_offset(res);
    printf("offset: %d\n", offset);
    res -= offset;

    printf("next_occurrence: %d\n", res);
    localtime_r(&res, &local_time);
    print_time("next_occurrence in human format: ", &local_time);

    return res;

}



int main(int argc, char * argv[]) {
    
    time_t curr_time, next;
    ws_entry schedule[MAX_ENTRIES];
    memset(schedule, 0, sizeof(ws_entry) * MAX_ENTRIES);

    //next monday at 11.40
    schedule[0].day_of_week = 1;
    schedule[0].start_time = (11*60 + 40);
 
    curr_time = time(NULL);
    printf("curr time: %d\n", curr_time);

    for (int i=0; i<53; i++) {
        printf("\n");
        next = next_occurrence(&schedule[0], curr_time + (i * SECS_PER_WEEK));
        printf("next: %d\n", next);
    }
}
