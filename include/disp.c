#include "disp.h"

void initDisplay(display_buf_t* display) {
	SHRClear();
    _delay_us(200);
    _setAnode(display, ANODE_ODD, ANODE_OFF);
	_setAnode(display, ANODE_EVEN, ANODE_ON);
	_setDigit(display, DIGIT_0, 0x00);
	_setDigit(display, DIGIT_1, 0x00);
	_setDigit(display, DIGIT_2, 0x00);
	_setDigit(display, DIGIT_3, 0x00);
	_setDigit(display, DIGIT_4, 0x00);
	_setDigit(display, DIGIT_5, 0x00);
	_sendBuffer(display);
}

void processDisplay(display_buf_t* display) {
	// blank for ~200us
	SHRClear();
	_delay_us(200);

    if (display->anode_even == ANODE_ON) {
        _setAnode(display, ANODE_ODD, ANODE_ON);
        _setAnode(display, ANODE_EVEN, ANODE_OFF);
    } else {
        _setAnode(display, ANODE_ODD, ANODE_OFF);
        _setAnode(display, ANODE_EVEN, ANODE_ON);
    }
    _sendBuffer(display);

//	// ** raw test pattern follows **
//    if (display->anode_even == ANODE_ON) {
//        // even anodes, show a 7
//        SHRSendByte(0x10);
//        SHRSendByte(0x00);
//        _setAnode(display, ANODE_EVEN, ANODE_OFF);
//        _setAnode(display, ANODE_ODD, ANODE_ON);
//    	SHRLatch();
//    } else {
//        // odd anodes, show an 8
//        SHRSendByte(0x21);
//        SHRSendByte(0x11);
//        _setAnode(display, ANODE_EVEN, ANODE_ON);
//        _setAnode(display, ANODE_ODD, ANODE_OFF);
//        SHRLatch();
//     }
}

// Just display some bytes
void displayDigits(display_buf_t* display, uint8_t* digits) {
	uint8_t i;

	for(i=0; i<6; i++) {
		_setDigit(display, i, digits[i]);
	}
}

void displayTime(display_buf_t* display, time_buf_t* time) {
	uint8_t tens, ones;

	ones = (time->seconds % 10);
	tens = (time->seconds - ones) / 10;

	_setDigit(display, DIGIT_0, ones);
	_setDigit(display, DIGIT_1, tens);

	ones = (time->minutes % 10);
	tens = (time->minutes - ones) / 10;

	_setDigit(display, DIGIT_2, ones);
	_setDigit(display, DIGIT_3, tens);

	ones = (time->hours % 10);
	tens = (time->hours - ones) / 10;

	_setDigit(display, DIGIT_4, ones);
	_setDigit(display, DIGIT_5, tens);
}

// TODO: optimize this so reversing isn't necessary
void _sendBuffer(display_buf_t* display) {
	// construct a bit buffer to clock out to the shift registers
	uint8_t rawbits[2] = {0x00, 0x00};
    uint8_t rawbits_reversed[2] = {0x00, 0x00};
	uint8_t i;

	if (display->anode_even == ANODE_ON) {
		// set anode bit
		rawbits[1] |= 0x08;

		// set digit driver inputs
		rawbits[0] |= ((_scrambleDigit(display->digit_0)) << 4);
		rawbits[0] |= _scrambleDigit(display->digit_2);
		rawbits[1] |= ((_scrambleDigit(display->digit_4)) << 4);
	} else {
		// set anode bit
		rawbits[1] |= 0x04;

		// set digit driver inputs
		rawbits[0] |= ((_scrambleDigit(display->digit_1)) << 4);
		rawbits[0] |= _scrambleDigit(display->digit_3);
		rawbits[1] |= ((_scrambleDigit(display->digit_5)) << 4);
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

// TODO: inline?
void _setAnode(display_buf_t* display, uint8_t anode, uint8_t value) {
	switch (anode) {
		case ANODE_EVEN:
			display->anode_even = value;
			break;
		case ANODE_ODD:
			display->anode_odd = value;
			break;
		default:
			break;
	}
}

// TODO: inline?
void _setDigit(display_buf_t* display, uint8_t digit, uint8_t value){
	switch (digit) {
		case DIGIT_0:
			display->digit_0 = value;
			break;
		case DIGIT_1:
			display->digit_1 = value;
			break;
		case DIGIT_2:
			display->digit_2 = value;
			break;
		case DIGIT_3:
			display->digit_3 = value;
			break;
		case DIGIT_4:
			display->digit_4 = value;
			break;
		case DIGIT_5:
			display->digit_5 = value;
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
			// return out-of-range code?
			return 0x00;
	}
}
