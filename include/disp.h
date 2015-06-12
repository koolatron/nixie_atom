/**
  * disp.h - display handling for nixie clock
  * Abstracts muxing, mangling, and clocking tasks
  */
#ifndef _DISP_H_
#define _DISP_H_

#include <stdint.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "time.h"
#include "shift.h"

#define ANODE_EVEN		0
#define ANODE_ODD		1
#define ANODE_ALL		2
#define ANODE_NONE		3
#define ANODE_OFF		0
#define ANODE_ON		1

#define DIGIT_0			0
#define DIGIT_1			1
#define DIGIT_2			2
#define DIGIT_3			3
#define DIGIT_4			4
#define DIGIT_5			5

#define BLANK_ON		0
#define BLANK_OFF		1

typedef struct {
	uint8_t digit_0;
	uint8_t digit_1;
	uint8_t digit_2;
	uint8_t digit_3;
	uint8_t digit_4;
	uint8_t digit_5;
	uint8_t anode_even;
	uint8_t anode_odd;
	uint8_t blank;
} display_buf_t;

void processDisplay(display_buf_t* display);
void initDisplay(display_buf_t* display);
void displayDigits(display_buf_t* display, uint8_t* digits);
void displayTime(display_buf_t* display, time_buf_t* time);
void blankDisplay(display_buf_t* display);
void unblankDisplay(display_buf_t* display);
void toggleBlank(display_buf_t* display);

void _sendBuffer(display_buf_t* display);
void _setAnode(display_buf_t* display, uint8_t anode, uint8_t value);
void _setDigit(display_buf_t* display, uint8_t digit, uint8_t value);
void _setBlank(display_buf_t* display, uint8_t blank);
uint8_t _scrambleDigit(uint8_t digit);

#endif
