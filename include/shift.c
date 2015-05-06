#include <stdint.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>

#include "shift.h"

void initSHR(void) {
    SHR_DDR  |=  (DATA | ENABLE | CLOCK | LATCH | CLEAR);
    SHR_PORT &= ~(DATA | ENABLE | CLOCK | LATCH | CLEAR);

    SHR_PORT |=  (CLEAR);

    SHREnable();
    SHRClear();
}

inline void SHRDisable(void) {
	SHR_PORT |= ENABLE;
	asm("nop");
}

inline void SHREnable(void) {
	SHR_PORT &= ~ENABLE; 
	asm("nop");
}

void SHRSendByte(unsigned char byte) {
	int8_t i;

	for (i = 7; i >= 0; i--) {
		SHR_PORT &= ~(DATA | CLOCK);
		asm("nop");
        if (byte & (1 << i)) {
            SHR_PORT |= DATA;
        } else {
            asm("nop");
        }
        asm("nop");
		SHR_PORT |= CLOCK;
		asm("nop");
	}

	SHR_PORT &= ~CLOCK;
	asm("nop");
}

inline void SHRLatch(void) {
	SHR_PORT |= LATCH;
	asm("nop");
	SHR_PORT &= ~LATCH;
	asm("nop");
}

void SHRClear(void) {
    SHR_PORT &= ~CLEAR;
    asm("nop");
    SHR_PORT |=  CLEAR;
    asm("nop");
}

