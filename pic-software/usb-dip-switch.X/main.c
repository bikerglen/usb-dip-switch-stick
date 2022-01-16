//-----------------------------------------------------------------------------------------------
// includes
//

#include "system.h"

#include "usb.h"
#include "usb_device_hid.h"

#include "app_device_custom_hid.h"


//-----------------------------------------------------------------------------------------------
// defines
//

enum {
    USB_SUSPENDED = 0,
    USB_CONNECTED = 1,
    USB_CONFIGURED = 2
};


//-----------------------------------------------------------------------------------------------
// typedefs
//


//-----------------------------------------------------------------------------------------------
// prototypes
//

uint8_t ProcessButton (uint8_t which, uint8_t sw);


//-----------------------------------------------------------------------------------------------
// globals
//

// flag from timer isr to main to execute 250 Hz / 4 ms tick
volatile uint8_t flag250 = 0;  

// 1.5 second period led timer counter
uint16_t ledTimer = 0;

// local variables for deciding to make a USB report or not
uint8_t reportNeeded;
uint8_t thisUsbReportData[1];
uint8_t lastUsbReportData[1];

// variables used to communicate report to USB ISR
volatile uint8_t usbReportNeeded = false;
volatile uint8_t usbReportData[1];
volatile uint8_t hostRequestedUsbReport = false;

// button debounce states
uint8_t buttonStates[8];


//-----------------------------------------------------------------------------------------------
// main
//

MAIN_RETURN main(void)
{
    uint8_t i;
    uint8_t newUsbState;
    
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);

    // no analog I/O
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    
#ifdef DEV_BOARD
    TRISCbits.TRISC3 = 0;
    LATCbits.LATC3 = 0;
    TRISCbits.TRISC5 = 1;
#else
    // initialize user LED to off
    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5 = 1;
    
    // initialize push button switch weak pullup and input
    WPUBbits.WPUB6 = 1;
    TRISBbits.TRISB6 = 1;
        
    // initialize dip switch inputs
    TRISAbits.TRISA5 = 1;
    TRISAbits.TRISA4 = 1;
    TRISCbits.TRISC5 = 1;
    TRISCbits.TRISC4 = 1;
    TRISBbits.TRISB7 = 1;
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 1;
    TRISCbits.TRISC3 = 1;
#endif
    
    // configure USB
    USBDeviceInit();
    USBDeviceAttach();
    
    // configure TMR2
    TMR2_Initialize ();

    // zero button states
	for (i = 0; i < 8; i++) {
		buttonStates[i] = 0;
	}
    
    // usb reporting variables
    reportNeeded = false;
    usbReportNeeded = false;
    for (i = 0; i < 1; i++) {
        thisUsbReportData[i] = 0;
        lastUsbReportData[i] = 0;
        usbReportData[i] = 0;
    }

    while(1) {
        SYSTEM_Tasks();

        #if defined(USB_POLLING)
            // Interrupt or polling method.  If using polling, must call
            // this function periodically.  This function will take care
            // of processing and responding to SETUP transactions
            // (such as during the enumeration process when you first
            // plug in).  USB hosts require that USB devices should accept
            // and process SETUP packets in a timely fashion.  Therefore,
            // when using polling, this function should be called
            // regularly (such as once every 1.8ms or faster** [see
            // inline code comments in usb_device.c for explanation when
            // "or faster" applies])  In most cases, the USBDeviceTasks()
            // function does not take very long to execute (ex: <100
            // instruction cycles) before it returns.
            USBDeviceTasks();
        #endif

        //Application specific tasks
        APP_DeviceCustomHIDTasks();
        
        
        // run 200 Hz tasks
        if (flag250) {
            // clear flag
            flag250 = 0;
            
            // get current USB state
            if (USBIsDeviceSuspended() == true) {
                newUsbState = USB_SUSPENDED;
            } else if (USBGetDeviceState() == CONFIGURED_STATE) {
                newUsbState = USB_CONFIGURED;
            } else {
                newUsbState = USB_CONNECTED;
            }
            
            // blink led
            if (ledTimer == 0) {
                USER_LED = LED_ON;
            } else if (ledTimer == 36) {
                USER_LED = LED_OFF;
            } else if ((ledTimer == 72) && (newUsbState >= USB_CONNECTED)) {
                USER_LED = LED_ON;
            } else if (ledTimer == 108) {
                USER_LED = LED_OFF;
            } else if ((ledTimer == 144) && (newUsbState >= USB_CONFIGURED)) {
                USER_LED = LED_ON;
            } else if (ledTimer == 180) {
                USER_LED = LED_OFF;
            }
            
            // increment led timer counter, 1.5 second period
            if (++ledTimer >= 375) {
                ledTimer = 0;
            }

			// clear USB report data
			for (i = 0; i < 1; i++) {
				thisUsbReportData[i] = 0;
			}

			// sample and process buttons
#ifdef DEV_BOARD        
			thisUsbReportData[0] |= ProcessButton (6, SW2);
#else
			thisUsbReportData[0] |= ProcessButton (7, SW1);
			thisUsbReportData[0] |= ProcessButton (6, SW2);
			thisUsbReportData[0] |= ProcessButton (5, SW3);
			thisUsbReportData[0] |= ProcessButton (4, SW4);
			thisUsbReportData[0] |= ProcessButton (3, SW5);
			thisUsbReportData[0] |= ProcessButton (2, SW6);
			thisUsbReportData[0] |= ProcessButton (1, SW7);
			thisUsbReportData[0] |= ProcessButton (0, SW8);
#endif

			// check if report needed
            reportNeeded = false;
            if (hostRequestedUsbReport) {
                hostRequestedUsbReport = false;
                reportNeeded = true;
            }
            if (thisUsbReportData[0] != lastUsbReportData[0]) {
                reportNeeded = true;
            }

			// save key presses for the next time around
			lastUsbReportData[0] = thisUsbReportData[0];

			// send report to USB code
			if (reportNeeded) {                
				usbReportNeeded = true;
				for (i = 0; i < 1; i++) {
					usbReportData[i] = thisUsbReportData[i];
				}
			}
        }        
    }
}


void TMR2_Initialize (void)
{
    PR2 = TMR2_PERIOD;
    TMR2 = 0x00;
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    T2CON = 0x5E;
}


void TMR2_InterruptHandler (void)
{
    PIR1bits.TMR2IF = 0;
    flag250 = 1;
}


uint8_t ProcessButton (uint8_t which, uint8_t sw)
{
	uint8_t state;

	state = buttonStates[which];

	switch (state) {
		case 0: state = sw ? 1 : 0; break;
		case 1: state = sw ? 2 : 0; break;
		case 2: state = sw ? 2 : 3; break;
		case 3: state = sw ? 2 : 0; break;
	}

	buttonStates[which] = state;

	return (state & 2) ? (1 << which) : 0;
}
