#include <stdint.h>
#include <stdbool.h>
//#include "clock.h"
#include "tm4c123gh6pm.h"

/**
 * main.c
 */


void wait1Second()
{
    __asm(".const");
    __asm("num .field 10000000");
    __asm("         LDR R0, num");
    __asm("loop:");
    __asm("         SUB R0, #1");
    __asm("         CBZ R0, end");
    __asm("         B loop");
    __asm("end:");
}

int main(void)
{
    wait1Second();
    while(1);
	//return 0;
}
