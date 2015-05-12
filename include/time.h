#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

#define TICKS_PER_SEC 	250
#define HOUR_MODE_12	0
#define HOUR_MODE_24 	1
#define NOT_COUNTING	0
#define IS_COUNTING		1
#define COUNT_UP		0
#define COUNT_DOWN		1

typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t ticks;
	uint8_t hour_mode;
	uint8_t is_counting;
	uint8_t count_dir;
} time_buf_t;

void initTime(time_buf_t* time);
void processTime(time_buf_t* time);

#endif
