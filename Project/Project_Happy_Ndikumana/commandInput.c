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
#include "commandInput.h"



//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initCommandHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
//    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
//    _delay_cycles(3);
//
//    // Configure LED pins
//    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | RED_LED_MASK;  // bits 1 and 3 are outputs
//    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | RED_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
//    GPIO_PORTF_DEN_R |= GREEN_LED_MASK | RED_LED_MASK;  // enable LEDs

    initUart0();
    setUart0BaudRate(115200, 40e6);
}

void  *memset(void *b, int c, int len)
{
  int i;
  unsigned char *p = b;
  i = 0;
  while(len > 0)
    {
      *p = c;
      p++;
      len--;
    }
  return(b);
}
void getsUart0(USER_DATA* data)
{
    uint8_t count = 0;
    //flush buffer
    memset(data->buffer,0,MAX_CHARS+1);

    while(count <= MAX_CHARS)
    {
        if(count == MAX_CHARS)
        {
            data->buffer[count+1] = '\0';
        }
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
    if(c > 47 && c < 58)
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

int getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
    if(data->fieldType[fieldNumber] == 'n')
    {
        char * temp = &data->buffer[data->fieldPosition[fieldNumber]];
        int sum = 0;
        int i = 0;
        while(temp[i] != '\0')
        {
            int currentInteger = temp[i] - 48;
            sum = (sum * 10) + currentInteger;
            i++;
        }
        return sum;
    }
    return 0;
}

//#define DEBUG

