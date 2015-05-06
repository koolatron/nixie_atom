#ifndef _SHIFT_H_
#define _SHIFT_H_

#define SHR_DDR  DDRD
#define SHR_PORT PORTD

#define DATA     (1 << 0)
#define ENABLE   (1 << 1)
#define LATCH    (1 << 2)
#define CLOCK    (1 << 3)
#define CLEAR    (1 << 4)

void initSHR(void);
void SHRSendByte(unsigned char byte);
void SHRDisable(void);
void SHREnable(void);
void SHRLatch(void);
void SHRClear(void);

#endif
