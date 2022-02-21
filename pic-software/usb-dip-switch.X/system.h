/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

// #define DEV_BOARD

#include <xc.h>
#include <stdbool.h>

#include "fixed_address_memory.h"

#define USE_INTERNAL_OSC

#define MAIN_RETURN void

// 250 Hz timer 2 period value
// dec2hex(12e6/16/12/250-1)
#define TMR2_PERIOD 0xF9

#ifdef DEV_BOARD
#define SW2      (PORTCbits.RC5 ? 0 : 1)
#define USER_LED LATCbits.LATC3
#define LED_ON 1
#define LED_OFF 0
#else

// GPIO input pins
#define PB1      (PORTBbits.RB6 ? 0 : 1)
#define SW1      (PORTAbits.RA5 ? 0 : 1)
#define SW2      (PORTAbits.RA4 ? 0 : 1)
#define SW3      (PORTCbits.RC5 ? 0 : 1)
#define SW4      (PORTCbits.RC4 ? 0 : 1)
#define SW5      (PORTBbits.RB7 ? 0 : 1)
#define SW6      (PORTCbits.RC7 ? 0 : 1)
#define SW7      (PORTCbits.RC6 ? 0 : 1)
#define SW8      (PORTCbits.RC3 ? 0 : 1)

// GPIO output pins
#define USER_LED LATBbits.LATB5

// led states
#define LED_ON   0
#define LED_OFF  1
#endif


/*** System States **************************************************/
typedef enum {
    SYSTEM_STATE_USB_START,
    SYSTEM_STATE_USB_SUSPEND,
    SYSTEM_STATE_USB_RESUME
} SYSTEM_STATE;

/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize( SYSTEM_STATE state );


/*********************************************************************
* Function: void SYSTEM_Tasks(void)
*
* Overview: Runs system level tasks that keep the system running
*
* PreCondition: System has been initialized with SYSTEM_Initialize()
*
* Input: None
*
* Output: None
*
********************************************************************/
//void SYSTEM_Tasks(void);
#define SYSTEM_Tasks()

void TMR2_Initialize (void);
void TMR2_InterruptHandler (void);

#endif //SYSTEM_H
