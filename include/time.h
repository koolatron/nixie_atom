#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

#define TICKS_PER_SEC 	250
#define HOUR_MODE_12	0
#define HOUR_MODE_24 	1
#define COUNT_ENABLED	0
#define COUNT_DISABLED	1
#define COUNT_UP		0
#define COUNT_DOWN		1

typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t ticks;
	uint8_t hour_mode;
	uint8_t count_en;
	uint8_t count_dir;
} time_buf_t;

void initTime(time_buf_t* time);
void processTime(time_buf_t* time);
void setHours(time_buf_t* time, uint8_t hours);
void setMinutes(time_buf_t* time, uint8_t minutes);
void setSeconds(time_buf_t* time, uint8_t seconds);

#endif
