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

uint8_t b1, b2, b3;
uint8_t state;

volatile uint8_t serviceUpdate;
volatile uint8_t timeUpdate, edges;

int main(void) {
    SetupHardware();

    /* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
    //CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

    GlobalInterruptEnable();

    timeBuffer.hours   = 00;
    timeBuffer.minutes = 00;
    timeBuffer.seconds = 00;

    state = STATE_CLOCK;

    for (;;) {
        // Ticks at ~250Hz
        if (serviceUpdate) {
            // clear update flag
            serviceUpdate = 0;

            if ((timeBuffer.ticks % 125) == 0) {
                LEDs_ToggleLEDs(LEDS_LED1);
            }

            if (!isLocked()) {
                timeBuffer.ticks %= TICKS_PER_SEC;
            }

            // Ticks at 1Hz via external source
            if (timeUpdate) {
                timeUpdate = 0;

                if (isLocked()) {
                    processTime(&timeBuffer);
                    LEDs_ToggleLEDs(LEDS_LED2);
                } else {
                    LEDs_TurnOnLEDs(LEDS_LED2);
                    toggleBlank(&displayBuffer);
                }
            }

            tick(&timeBuffer);
            displayTime(&displayBuffer, &timeBuffer);

            processDisplay(&displayBuffer);
            processButtons();
            processState();
        }

        //CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        //USB_USBTask();
    };
}

/** Configures the board hardware and chip peripherals. */
void SetupHardware(void)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    //USB_Init();
    Buttons_Init();
    LEDs_Init();
    initSHR();
    initDisplay(&displayBuffer);
    initTime(&timeBuffer);

    /* Initialize timer0 */
    /* This sets up a timer interrupt at 250Hz to signal display service */
    OCR0A   = 249;                            // Top count; timer0 interrupt rate = 16MHz / (256 * (OCR0A + 1))
    TCCR0A |=  (1 << WGM01);                  // CTC mode
    TIMSK0 |=  (1 << OCIE0A);                 // Enable interrupt generation on OCR0A match
    TCCR0B |=  (1 << CS02);                   // F/256 prescaler; start timer

    /* Initialize PCINT */
    /* This sets up a pin-change interrupt to catch the 1PPS output of our rubidium source */
    DDRC   &= ~(1 << PC2);                    // PORTC[2] is an input
    PORTC  &= ~(1 << PC2);                    // Weak pullup on PORTC[2] disabled
    PCMSK1 |=  (1 << PCINT11);                // Enable PCINT11 interrupt
    PCICR  |=  (1 << PCIE1);                  // Enable PCI1 interrupt generation on pin state change

    /* Initialize !LOCK input pin */
    DDRC   &= ~(1 << PC4);                    // PORTC[4] is an input
    PORTC  &= ~(1 << PC4);                    // Weak pullup on PORTC[4] disabled
}

void processButtons(void) {
    uint8_t button_status = Buttons_GetStatus();

    if (button_status & BUTTONS_BUTTON1) {
        if (b1 > BUTTON_ON) {
            b1 = BUTTON_OFF;
        } else {
            b1++;
        }
    }

    if (button_status & BUTTONS_BUTTON2) {
        if (b2 > BUTTON_ON) {
            b2 = BUTTON_OFF;
        } else {
            b2++;
        }
    }

    if (button_status & BUTTONS_BUTTON3) {
        if (b3 > BUTTON_ON) {
            b3 = BUTTON_OFF;
        } else {
            b3++;
        }
    }
}

// Business logic
void processState(void) {
    switch(state) {
        case STATE_CLOCK:
            if (b1 == BUTTON_ON)
                toggleCount(&timeBuffer);
            if (b2 == BUTTON_ON)
                toggleCountDir(&timeBuffer);
            if (b3 == BUTTON_ON)
                state = STATE_CLOCK_SET;
            break;

        case STATE_CLOCK_SET:
            disableCount(&timeBuffer);

            if (b1 == BUTTON_ON)
                nextMinute(&timeBuffer);
            if (b2 == BUTTON_ON)
                nextHour(&timeBuffer);
            if (b3 == BUTTON_ON)
                state = STATE_CLOCK;
            break;
        default:
            break;
    }
}

inline uint8_t isLocked(void) {
    if (PINC & (1 << PC4)) {
        return 0;
    }
    return 1;
}

ISR(TIMER0_COMPA_vect) {
    serviceUpdate = 1;
}

ISR(PCINT1_vect) {
    edges++;
    if (edges > 1) {
        timeUpdate = 1;
        edges = 0;
    }
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
