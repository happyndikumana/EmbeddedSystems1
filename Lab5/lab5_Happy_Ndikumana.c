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
int isAlpha(char c)
{
    if((c > 64 && c < 91) || (c > 96 && c < 123))
        return 1;
    return 0;
}
int isNum(char c)
{
    if(c > 47 && c < 57)
        return 1;
    return 0;
}
void parseFields(USER_DATA* data)
{
    //loop until null terminator
    //bool firstSeen Var = true when buffer[i] != num | alpha

    uint8_t i = 0;
    int seen = 0;
    data->fieldCount = 0;
    while(data->buffer[i] != '\0') //read until null terminator. Don't read the whole buffer
    {
        if(isAlpha(data->buffer[i]) || isNum(data->buffer[i]))
        {
            if(seen == 0) //new word or number
            {
                data->fieldPosition[data->fieldCount] = i;
                if(isNum(data->buffer[i]))
                {
                    data->fieldType[data->fieldCount] = 'n';
                }
                else
                {
                    data->fieldType[data->fieldCount] = 'a';
                }
                data->fieldCount++;

            }
            seen = 1;
        }
        else
        {
            seen = 0;
            data->buffer[i] = '\0';
        }
        if(data->fieldCount == MAX_FIELDS)
            return;
        i++;
    }
}

bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    uint8_t i = 0;
    char * temp = &data->buffer[data->fieldPosition[0]];
    for(i = 0; temp[i] != '\0'; i++)
    {
        if(strCommand[i] != temp[i])
            return false;
    }
    if(data->fieldCount <= minArguments)
        return false;

    return true;
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    if(data->fieldType[fieldNumber] == 'a')
        return &data->buffer[data->fieldPosition[fieldNumber]];

    return 0;
}

int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
    if(data->fieldType[fieldNumber] == 'n')
    {
        char * temp = &data->buffer[data->fieldPosition[fieldNumber]];
        int32_t sum = 0;
        int i = 0;
        while(temp[i] != '\0')
        {
            int32_t currentInteger = temp[i] - 48;
            sum = (sum * 10) + currentInteger;
            i++;
        }
        return sum;
    }
    return 0;
}
//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

#define DEBUG

int main(void)
{
    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);

    USER_DATA data;

    putsUart0("> ");
    // Get the string from the user
    getsUart0(&data);
    // Echo back to the user of the TTY interface for testing
    #ifdef DEBUG
    putsUart0(data.buffer);
    putcUart0('\n');
    #endif
    // Parse fields
    //parseFields(&data);
    parseFields(&data);
    // Echo back the parsed field data (type and fields)
    #ifdef DEBUG
    uint8_t i;
    for (i = 0; i < data.fieldCount; i++)
    {
        putcUart0(data.fieldType[i]);
        putcUart0('\t');
        putsUart0(&data.buffer[data.fieldPosition[i]]);
        putcUart0('\n');
    }
    #endif
    // Command evaluation
    // set add, data → add and data are integers
    bool valid = false;
    if (isCommand(&data, "set", 2))
    {
        int32_t add = getFieldInteger(&data, 1);
        int32_t num = getFieldInteger(&data, 2);
        //int32_t data = getFieldInteger(&data,2);
        valid = true;
    // do something with this information
    }
    // alert ON|OFF → alert ON or alert OFF are the expected commands
    if (isCommand(&data, "alert", 1))
    {
        char* str = getFieldString(&data, 1);
        valid = true;
    // process the string with your custom strcmp instruction, then do something
    }
    // Process other commands here
    // Look for error
    if (!valid)
        putsUart0("Invalid command\n");



//    while(true)
//    {
//        putsUart0("Enter smth: ");
//        getsUart0(&data);
//        putsUart0(data.buffer);
//        putsUart0("\n");
//    }
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
