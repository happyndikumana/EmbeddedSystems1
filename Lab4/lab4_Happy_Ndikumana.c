// Serial Example
// Jason Losh and Happy Ndikumana

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
//#include <string.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

// Bitband aliases
#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// PortF masks
#define GREEN_LED_MASK 8
#define RED_LED_MASK 2


#define MAX_CHARS 80
#define MAX_FIELDS 5
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    // Configure LED pins
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | RED_LED_MASK;  // bits 1 and 3 are outputs
    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | RED_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= GREEN_LED_MASK | RED_LED_MASK;  // enable LEDs
}
void getsUart0(USER_DATA* data)
{
    uint8_t count = 0;

    while(count < MAX_CHARS)
    {
        char input = getcUart0();

        if(input >= 32 && input != 127)
        {
            data->buffer[count] = input;
            count++;
        }
        if((input == 8 || input == 127) && count > 0)
        {
            count--;
        }
        if(input == 13)
        {
            data->buffer[count] = '\0';
            return;
        }
    }
    return;
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    initHw();
    initUart0();
    setUart0BaudRate(19200, 40e6);
    USER_DATA data;

    while(true)
    {
        putsUart0("Enter smth: ");
        getsUart0(&data);
        putsUart0(data.buffer);
        putsUart0("\n");
    }
    //return 0;


	// Initialize hardware
//	initHw();
//	initUart0();
//
//    // Setup UART0 baud rate
//    //setUart0BaudRate(115200, 40e6);
//
//	// Display greeting
//    putsUart0("Serial Example\n");
//    putsUart0("Press '0' or '1'\n");
//    putcUart0('>');
//
//    // For each received character, toggle the green LED
//    // For each received "1", set the red LED
//    // For each received "0", clear the red LED
//    while(true)
//    {
//    	char c = getcUart0();
//    	GREEN_LED ^= 1;
//    	if (c == '1')
//    	    RED_LED = 1;
//    	if (c == '0')
//    		RED_LED = 0;
//    }
}
