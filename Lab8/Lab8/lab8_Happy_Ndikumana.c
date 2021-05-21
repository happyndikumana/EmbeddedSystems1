#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "pedometer.h"
#include "motor.h"
#include "uart0.h"

#define SLEEP (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 3*4)))


int main()
{
    initSystemClockTo40Mhz();
    initMotor();
    initPedometer();
    initUart0();
    char out[50];
    //int count1, count2;
    putsUart0("New Start\n");
    waitMicrosecond(2000000);
    SLEEP = 1;

    //wheel diameter = 6.476 cm
    //Circumference = 2*pi*r = 2*pi*(6.476/2);
    //ratio of wheel and white gear: 1:46. one wheel rotation = 46 white gear rotations
    //22cm = 44 counts
    //19.3cm = 43 counts
    //18.6cm = 41 counts <-- at 0 = 90, 1 = 80
    //0 - right, 1 - left

    int count0, count1;
    setEncoderPosition(1, 0);
    setEncoderPosition(0, 0);

    while(1)
    {
        reverse(100);
        SLEEP = 0;
        waitMicrosecond(1000000);
        SLEEP = 1;

        rotateCW(90);
        SLEEP = 0;
        waitMicrosecond(1000000);
        SLEEP = 1;

        reverse(70);
        SLEEP = 0;
        waitMicrosecond(1000000);
        SLEEP = 1;

        forward(70);
        SLEEP = 0;
        waitMicrosecond(1000000);
        SLEEP = 1;

        rotateCCW(90);
        SLEEP = 0;
        waitMicrosecond(1000000);
        SLEEP = 1;
////////
        forward(100);
        SLEEP = 0;
        waitMicrosecond(1000000);
        SLEEP = 1;
    }
}
