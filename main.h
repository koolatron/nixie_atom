#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Descriptors.h"

#include "include/shift.h"
#include "include/disp.h"
#include "include/time.h"

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/Board/Buttons.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

//#define CLOCK_SOURCE_EXT			// Comment out to use internal clock source

#define FLASH_SIZE_BYTES            0x8000
#define BOOTLOADER_SEC_SIZE_BYTES   0x1000
#define BOOTLOADER_START_ADDRESS    (FLASH_SIZE_BYTES - BOOTLOADER_SEC_SIZE_BYTES)
#define MAGIC_BOOT_KEY              0x5CA1AB1E

#define LEDMASK_USB_ENUMERATING     LEDS_LED1
#define LEDMASK_USB_NOTREADY        LEDS_LED2
#define LEDMASK_USB_ERROR           LEDS_ALL_LEDS
#define LEDMASK_USB_READY           LEDS_LED1

#define STATE_LOGIC_NOT_LOCKED		0
#define STATE_LOGIC_COUNT           1
#define STATE_LOGIC_SET             2
#define STATE_LOGIC_SLOTMACHINE		3

#define STATE_LOCK_TRUE             5
#define STATE_LOCK_FALSE            6

#define UPDATE_RATE_VERY_FAST		27		// rates in ms/4 (100 = 400ms delay)
#define UPDATE_RATE_FAST			62
#define UPDATE_RATE_SLOW			150
#define UPDATE_RATE_VERY_SLOW		250

#define BUTTON_ON                   0
#define BUTTON_OFF                  25		// button delay in ms/4

#define RNG_STATE_BYTES 			10
#define RNG_MULT 					0x153
#define RNG_MULT_LO					(RNG_MULT & 255)
#define RNG_MULT_HI					(RNG_MULT & 256)

typedef struct {
	uint8_t logic;
	uint8_t buttons;
	uint8_t lock;

	uint8_t update_rate;
	uint8_t update_rate_counter;
} state_buf_t;

void processButtons(void);
void processState(void);
inline uint8_t isLocked(void);
uint8_t rand8(void);

/** The ATTR_INIT_SECTION(3) decoration causes gcc to place this function in the .init3 section,
 *  which runs before main but after the stack is initialized.
 */
void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void Jump_To_Bootloader(void);

void initState(state_buf_t *stateBuffer);
void SetupHardware(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
