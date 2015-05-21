#include "time.h"

void initTime(time_buf_t* time) {
    time->ticks = 0;
    time->seconds = 0;
    time->minutes = 0;
    time->hours = 0;
    time->hour_mode = HOUR_MODE_24;
    time->count_en = COUNT_ENABLED;
    time->count_dir = COUNT_UP;
}

void tick(time_buf_t* time) {
    if (time->count_en == COUNT_DISABLED)
        return;

    time->ticks++;
}

void processTime(time_buf_t* time) {
    if (time->count_en == COUNT_DISABLED)
        return;

    if (time->count_dir == COUNT_UP) {
        time->seconds++;
        time->ticks = 0;

    	if (time->seconds >= 60) {
    		time->minutes++;
    		time->seconds = 0;
    	}

    	if (time->minutes >= 60) {
    		time->hours++;
    		time->minutes = 0;
    	}

    	if (time->hour_mode == HOUR_MODE_12) {
    		if (time->hours >= 13)
    			time->hours = 1;
    	}

    	if (time->hour_mode == HOUR_MODE_24) {
    		if (time->hours >= 24)
    			time->hours = 0;
    	}
    }

    if (time->count_dir == COUNT_DOWN) {
        time->seconds--;
        time->ticks = 0;

        if (time->seconds >= 60) {
            time->minutes--;
            time->seconds = 59;
        }

        if (time->minutes >= 60) {
            time->hours--;
            time->minutes = 59;
        }

        if ((time->hours == 0) && (time->minutes == 0) && (time->seconds == 0))
            time->count_en == COUNT_DISABLED;
    }
}
