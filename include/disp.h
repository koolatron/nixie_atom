/**
  * disp.h - display handling for nixie clock
  * Abstracts muxing, mangling, and clocking tasks
  */

#define ANODE_EVEN		0
#define ANODE_ODD		1
#define ANODE_OFF		0
#define ANODE_ON		1

#define DIGIT_0			0
#define DIGIT_1			1
#define DIGIT_2			2
#define DIGIT_3			3
#define DIGIT_4			4
#define DIGIT_5			5

typedef struct {
	uint8_t digit_0;
	uint8_t digit_1;
	uint8_t digit_2;
	uint8_t digit_3;
	uint8_t digit_4;
	uint8_t digit_5;
	uint8_t anode_even;
	uint8_t anode_odd;
} display_buf_t;

void processDisplay(display_buf_t* buffer);
void initDisplay(display_buf_t* buffer);
void _sendBuffer(display_buf_t* buffer);
void _setAnode(display_buf_t* buffer, uint8_t anode, uint8_t value);
void _setDigit(display_buf_t* buffer, uint8_t digit, uint8_t value);
uint8_t _scrambleDigit(uint8_t digit);
