#include <stdint.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "disp.h"
#include "shift.h"

void initDisplay(display_buf_t* buffer) {
	SHRClear();
	_setAnode(buffer, ANODE_ODD, ANODE_OFF);
	_setAnode(buffer, ANODE_EVEN, ANODE_ON);
	_setDigit(buffer, DIGIT_0, 0);
	_setDigit(buffer, DIGIT_0, 0);
	_setDigit(buffer, DIGIT_0, 0);
	_setDigit(buffer, DIGIT_0, 0);
	_setDigit(buffer, DIGIT_0, 0);
	_setDigit(buffer, DIGIT_0, 0);
	_sendBuffer(buffer);
}

void processDisplay(display_buf_t* buffer) {
	uint8_t anodeState = buffer->anode_even;

	// blank for ~200us
	SHRClear();
	_delay_us(200);

	// anode on
	if (anodeState == ANODE_ON) {
		_setAnode(buffer, ANODE_ODD, ANODE_ON);
	} else {
		_setAnode(buffer, ANODE_EVEN, ANODE_ON);
	}
	_sendBuffer(buffer);

	// ** test pattern follows
	// even digits on, digit outputs set to 7
	//SHRSendByte(0x10);
	//SHRSendByte(0x00);
	//SHRLatch();
}

void _sendBuffer(display_buf_t* buffer) {
	// Scramble digits, reverse bit chain, clock it out
	uint8_t rawbits[2];
	uint8_t i;

	if (buffer->anode_even == ANODE_ON) {
		// set anode bit
		rawbits[1] |= 0x08;

		// set digit driver inputs
		rawbits[0] |= ((_scrambleDigit(buffer->digit_0)) << 4);
		rawbits[0] |= _scrambleDigit(buffer->digit_2);
		rawbits[1] |= ((_scrambleDigit(buffer->digit_4)) << 4);
	}
	if (buffer->anode_odd == ANODE_ON) {
		// set anode bit
		rawbits[1] |= 0x04;

		// set digit driver inputs
		rawbits[0] |= ((_scrambleDigit(buffer->digit_1)) << 4);
		rawbits[0] |= _scrambleDigit(buffer->digit_3);
		rawbits[1] |= ((_scrambleDigit(buffer->digit_5)) << 4);
	}

	// reverse the bits so they're in the right order to clock out
	for (i=0; i<8; i++) {
		rawbits[0] |= ((rawbits[0] >> i) & 1) << (7-i);
		rawbits[1] |= ((rawbits[1] >> i) & 1) << (7-i);
	}

	SHRSendByte(rawbits[1]);
	SHRSendByte(rawbits[0]);
	SHRLatch();
}

void _setAnode(display_buf_t* buffer, uint8_t anode, uint8_t value) {
	switch (anode) {
		case ANODE_EVEN:
			buffer->anode_even = value;
			break;
		case ANODE_ODD:
			buffer->anode_odd = value;
			break;
		default:
			break;
	}
}

void _setDigit(display_buf_t* buffer, uint8_t digit, uint8_t value){
	switch (digit) {
		case DIGIT_0:
			buffer->digit_0 = value;
		case DIGIT_1:
			buffer->digit_1 = value;
		case DIGIT_2:
			buffer->digit_2 = value;
		case DIGIT_3:
			buffer->digit_3 = value;
		case DIGIT_4:
			buffer->digit_4 = value;
		case DIGIT_5:
			buffer->digit_5 = value;
		default:
			break;
	}
}

uint8_t _scrambleDigit(uint8_t digit) {
	switch (digit) {
		case 0:
			return 3;
		case 1:
			return 8;
		case 2:
			return 9;
		case 3:
			return 5;
		case 4:
			return 4;
		case 5:
			return 6;
		case 6:
			return 7;
		case 7:
			return 0;
		case 8:
			return 1;
		case 9:
			return 2;
		default:
			return 0;
	}
}
