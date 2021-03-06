#include "main.h"

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
    {
        .Config =
            {
                .ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
                .DataINEndpoint           =
                    {
                        .Address          = CDC_TX_EPADDR,
                        .Size             = CDC_TXRX_EPSIZE,
                        .Banks            = 1,
                    },
                .DataOUTEndpoint =
                    {
                        .Address          = CDC_RX_EPADDR,
                        .Size             = CDC_TXRX_EPSIZE,
                        .Banks            = 1,
                    },
                .NotificationEndpoint =
                    {
                        .Address          = CDC_NOTIFICATION_EPADDR,
                        .Size             = CDC_NOTIFICATION_EPSIZE,
                        .Banks            = 1,
                    },
            },
    };

uint32_t Boot_Key ATTR_NO_INIT;

//static FILE USBSerialStream;
static time_buf_t timeBuffer;
static display_buf_t displayBuffer;
static state_buf_t stateBuffer;

uint8_t b1, b2, b3;

volatile uint8_t serviceUpdate;
volatile uint8_t timeUpdate, edges;

int main(void) {
    // Hardware initialization
    SetupHardware();

    /* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
    //CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

    // Buffer initialization
    initState(&stateBuffer);
    initDisplay(&displayBuffer);
    initTime(&timeBuffer);

    GlobalInterruptEnable();

    for (;;) {
        // Ticks at ~250Hz
        if (serviceUpdate) {
            serviceUpdate = 0;

            // Ticks at 1Hz via external interrupt
            if (timeUpdate) {
                timeUpdate = 0;

                processTime(&timeBuffer);
                LEDs_ToggleLEDs(LEDS_LED2);
            }

#ifndef CLOCK_SOURCE_EXT
            // tick() will set timeBuffer.ticks to 0 when it reaches TICKS_PER_SEC
            if (timeBuffer.ticks == 0) {
                timeUpdate = 1;
            }
#endif // CLOCK_SOURCE_EXT

            tick(&timeBuffer);

            processButtons();
            processState();
            processDisplay(&displayBuffer);
        }

        //CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        //USB_USBTask();
    };
}

void initState(state_buf_t* stateBuffer) {
    stateBuffer->logic = STATE_LOGIC_NOT_LOCKED;
    stateBuffer->lock = STATE_LOCK_FALSE;
    stateBuffer->update_rate = UPDATE_RATE_VERY_FAST;
    stateBuffer->update_rate_counter = 0;
}

/** Configures the board hardware and chip peripherals. */
void SetupHardware(void)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* General Initialization */
    //USB_Init();
    Buttons_Init();
    LEDs_Init();
    initSHR();

    /* Initialize timer0 */
    /* This sets up a timer interrupt at 250Hz to signal display service */
    OCR0A   = 249;                            // Top count; timer0 interrupt rate = 16MHz / (256 * (OCR0A + 1))
    TCCR0A |=  (1 << WGM01);                  // CTC mode
    TIMSK0 |=  (1 << OCIE0A);                 // Enable interrupt generation on OCR0A match
    TCCR0B |=  (1 << CS02);                   // F/256 prescaler; start timer

    /* Initialize PCINT */
    /* This sets up a pin-change interrupt to catch the 1PPS output of our rubidium source */
#ifdef CLOCK_SOURCE_EXT
    DDRC   &= ~(1 << PC2);                    // PORTC[2] is an input
    PORTC  &= ~(1 << PC2);                    // Weak pullup on PORTC[2] disabled
    PCMSK1 |=  (1 << PCINT11);                // Enable PCINT11 interrupt
    PCICR  |=  (1 << PCIE1);                  // Enable PCI1 interrupt generation on pin state change
#endif // CLOCK_SOURCE_EXT

    /* Initialize !LOCK input pin */
    DDRC   &= ~(1 << PC4);                    // PORTC[4] is an input
    PORTC  &= ~(1 << PC4);                    // Weak pullup on PORTC[4] disabled
}

void processButtons(void) {
    uint8_t button_status = Buttons_GetStatus();

    if (button_status & BUTTONS_BUTTON1) {
        if (b1 > BUTTON_ON)
          b1--;
    } else {
        if (b1 < BUTTON_OFF)
          b1++;
    }

    if (button_status & BUTTONS_BUTTON2) {
        if (b2 > BUTTON_ON)
          b2--;
    } else {
        if (b2 < BUTTON_OFF)
          b2++;
    }

    if (button_status & BUTTONS_BUTTON3) {
        if (b3 > BUTTON_ON)
          b3--;
    } else {
        if (b3 < BUTTON_OFF)
          b3++;
    }
}

// Business logic
void processState(void) {
    switch(stateBuffer.logic) {
        case STATE_LOGIC_NOT_LOCKED:
            disableCount(&timeBuffer);

            displayBuffer.flash = FLASH_ON;
            displayBuffer.flash_rate = FLASH_RATE_SLOW;

            stateBuffer.lock = STATE_LOCK_FALSE;

            LEDs_SetAllLEDs(LEDS_LED1);

            if (isLocked()) {
                stateBuffer.logic = STATE_LOGIC_COUNT;
                stateBuffer.lock = STATE_LOCK_TRUE;
            }
            break;
        case STATE_LOGIC_COUNT:
            displayBuffer.flash = FLASH_OFF;

            if (b1 == BUTTON_ON) {
                b1 = BUTTON_OFF;
                toggleCount(&timeBuffer);
            }
            if (b2 == BUTTON_ON) {
                b2 = BUTTON_OFF;
                toggleCountDir(&timeBuffer);
            }
            if (b3 == BUTTON_ON) {
                b3 = BUTTON_OFF;
                stateBuffer.logic = STATE_LOGIC_SET;

                timeBuffer.hours = 0;
                timeBuffer.minutes = 0;
                timeBuffer.seconds = 0;
            }

            // Countdown complete condition
            if (timeBuffer.count_dir == COUNT_DOWN) {
                if ((timeBuffer.hours   == 0) &&
                    (timeBuffer.minutes == 0) &&
                    (timeBuffer.seconds == 0)) {
                        stateBuffer.logic = STATE_LOGIC_SLOTMACHINE;
                }
            }
            break;
        case STATE_LOGIC_SET:
            disableCount(&timeBuffer);

            displayBuffer.flash = FLASH_ON;
            displayBuffer.flash_rate = FLASH_RATE_FAST;

            if (b1 == BUTTON_ON) {
                b1 = BUTTON_OFF;
                nextHour(&timeBuffer);
            }
            if (b2 == BUTTON_ON) {
                b2 = BUTTON_OFF;
                nextMinute(&timeBuffer);
            }
            if (b3 == BUTTON_ON) {
                b3 = BUTTON_OFF;
                stateBuffer.logic = STATE_LOGIC_COUNT;
            }
            break;
        case STATE_LOGIC_SLOTMACHINE:
            disableCount(&timeBuffer);

            displayBuffer.flash = FLASH_OFF;
            stateBuffer.update_rate_counter++;

            if ((stateBuffer.update_rate_counter % stateBuffer.update_rate) == 0) {
                stateBuffer.update_rate_counter = 0;

                timeBuffer.seconds = rand8() % 100;
                timeBuffer.minutes = rand8() % 100;
                timeBuffer.hours   = rand8() % 100;
            }

            if (b3 == BUTTON_ON) {
                b3 = BUTTON_OFF;

                timeBuffer.seconds = 0;
                timeBuffer.minutes = 0;
                timeBuffer.hours   = 0;

                stateBuffer.logic = STATE_LOGIC_COUNT;
                timeBuffer.count_dir = COUNT_UP;
            }
            break;
        default:
            break;
    }

    displayTime(&displayBuffer, &timeBuffer);
}

inline uint8_t isLocked(void) {
#ifdef CLOCK_SOURCE_EXT
    if (PINC & (1 << PC4)) {
        return 0;
    }
#endif // CLOCK_SOURCE_EXT
    return 1;
}

/** simple LFSR-based PRNG that's faster and smaller than rand()
    Using this shaves off about 400 bytes of program code
*/
uint8_t rand8(void) {
    static uint8_t state[RNG_STATE_BYTES] =
        { 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c, 0xb6, 0xca, 0x0a };
    static uint16_t c = 0x42;
    static uint8_t  i = 0;
    uint16_t t;
    uint8_t x;

    x = state[i];
    t = (uint16_t)x * RNG_MULT_LO + c;
    c = t >> 8;
#if RNG_MULT_HI
    c += x;
#endif
    x = t & 255;
    state[i] = x;
    if (++i >= sizeof(state))
        i = 0;

    return x;
}

/** Timer interrupt triggers every 4ms */
ISR(TIMER0_COMPA_vect) {
    serviceUpdate = 1;
}

/** Pin-change interrupt triggers twice per second */
ISR(PCINT1_vect) {
#ifdef CLOCK_SOURCE_EXT
    edges++;
    if (edges > 1) {
        timeUpdate = 1;
        edges = 0;
    }
#endif // CLOCK_SOURCE_EXT
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;
    ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
    LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}


/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void Bootloader_Jump_Check(void)
{
    // If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
    if ((MCUSR & (1 << WDRF)) && (Boot_Key == MAGIC_BOOT_KEY))
    {
        Boot_Key = 0;
        ((void (*)(void))BOOTLOADER_START_ADDRESS)();
    }
}

void Jump_To_Bootloader(void)
{
    // If USB is used, detach from the bus and reset it
    USB_Disable();

    // Disable all interrupts
    cli();

    // Wait two seconds for the USB detachment to register on the host
    Delay_MS(2000);

    // Set the bootloader key to the magic value and force a reset
    Boot_Key = MAGIC_BOOT_KEY;
    wdt_enable(WDTO_250MS);
    for (;;);
}
