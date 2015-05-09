#include <stdint.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "disp.h"
#include "shift.h"

void initDisplay(display_buf_t* buffer) {
	SHRClear();
    _delay_us(200);
    _setAnode(buffer, ANODE_ODD, ANODE_OFF);
	_setAnode(buffer, ANODE_EVEN, ANODE_ON);
	_setDigit(buffer, DIGIT_0, 0x00);
	_setDigit(buffer, DIGIT_1, 0x00);
	_setDigit(buffer, DIGIT_2, 0x00);
	_setDigit(buffer, DIGIT_3, 0x00);
	_setDigit(buffer, DIGIT_4, 0x00);
	_setDigit(buffer, DIGIT_5, 0x00);
	_sendBuffer(buffer);
}

void processDisplay(display_buf_t* buffer) {
	// blank for ~200us
	SHRClear();
	_delay_us(200);

    if (buffer->anode_even == ANODE_ON) {
        _setAnode(buffer, ANODE_ODD, ANODE_ON);
        _setAnode(buffer, ANODE_EVEN, ANODE_OFF);
    } else {
        _setAnode(buffer, ANODE_ODD, ANODE_OFF);
        _setAnode(buffer, ANODE_EVEN, ANODE_ON);
    }
    _sendBuffer(buffer);

//	// ** test pattern follows (working) **
//    if (buffer->anode_even == ANODE_ON) {
//        // even anodes, show a 7
//        SHRSendByte(0x10);
//        SHRSendByte(0x00);
//        _setAnode(buffer, ANODE_EVEN, ANODE_OFF);
//        _setAnode(buffer, ANODE_ODD, ANODE_ON);
//    	SHRLatch();
//    } else {
//        // odd anodes, show an 8
//        SHRSendByte(0x21);
//        SHRSendByte(0x11);
//        _setAnode(buffer, ANODE_EVEN, ANODE_ON);
//        _setAnode(buffer, ANODE_ODD, ANODE_OFF);
//        SHRLatch();
//     }
}

void _sendBuffer(display_buf_t* buffer) {
	// construct a bit buffer to clock out to the shift registers
	uint8_t rawbits[2] = {0x00, 0x00};
    uint8_t rawbits_reversed[2] = {0x00, 0x00};
	uint8_t i;

	if (buffer->anode_even == ANODE_ON) {
		// set anode bit
		rawbits[1] |= 0x08;

		// set digit driver inputs
		rawbits[0] |= ((_scrambleDigit(buffer->digit_0)) << 4);
		rawbits[0] |= _scrambleDigit(buffer->digit_2);
		rawbits[1] |= ((_scrambleDigit(buffer->digit_4)) << 4);
	} else {
		// set anode bit
		rawbits[1] |= 0x04;

		// set digit driver inputs
		rawbits[0] |= ((_scrambleDigit(buffer->digit_1)) << 4);
		rawbits[0] |= _scrambleDigit(buffer->digit_3);
		rawbits[1] |= ((_scrambleDigit(buffer->digit_5)) << 4);
	}

	// reverse the bits so they're in the right order to clock out
	for (i=0; i<8; i++) {
		rawbits_reversed[0] |= ((rawbits[0] >> i) & 1) << (7-i);
		rawbits_reversed[1] |= ((rawbits[1] >> i) & 1) << (7-i);
	}

    // clock out the buffer we built up
	SHRSendByte(rawbits_reversed[1]);
	SHRSendByte(rawbits_reversed[0]);
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
			break;
		case DIGIT_1:
			buffer->digit_1 = value;
			break;
		case DIGIT_2:
			buffer->digit_2 = value;
			break;
		case DIGIT_3:
			buffer->digit_3 = value;
			break;
		case DIGIT_4:
			buffer->digit_4 = value;
			break;
		case DIGIT_5:
			buffer->digit_5 = value;
			break;
		default:
			break;
	}
}

// TODO: do this with a LUT and not a function
uint8_t _scrambleDigit(uint8_t digit) {
	switch (digit) {
		case 0x00:
			return 0x0c;
		case 0x01:
			return 0x01;
		case 0x02:
			return 0x09;
		case 0x03:
			return 0x0a;
		case 0x04:
			return 0x02;
		case 0x05:
			return 0x06;
		case 0x06:
			return 0x0e;
		case 0x07:
			return 0x00;
		case 0x08:
			return 0x08;
		case 0x09:
			return 0x04;
		default:
			return 0x00;
	}
}
