/*
 * timer0.c
 *
 *  Created on: Apr 13, 2021
 *      Author: Sarker Nadir Afridi Azmi
 *
 *  This timer controls the synching of devices connected to the network
 */

#include <stdint.h>
#include <stdbool.h>
#include "timer0.h"
#include "clock.h"
#include "uart0.h"

#define TRIGGER (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4)))
#define ECHO    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4)))

#define TRIGGER_MASK 8
#define ECHO_MASK 4

void waitMicrosecond(uint32_t us)
{
    __asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*2 (speculative, so P=1)
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             NOP");                  // 1
    __asm("             B    WMS_LOOP0");       // 1*2 (speculative, so P=1)
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}

void initTimer0()
{
    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);

    // Configure GPO pin
    GPIO_PORTE_DIR_R |= TRIGGER_MASK;
    GPIO_PORTE_DEN_R |= TRIGGER_MASK;

    GPIO_PORTE_DIR_R &= ~ECHO_MASK;
    GPIO_PORTE_DEN_R |= ECHO_MASK;

    // Configure Timer 1 as the time base
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;          // configure for oneshot mode (count up)
    TIMER0_CTL_R |= TIMER_CTL_TAEN;
    // TIMER0_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    // NVIC_EN0_R |= 1 << (INT_TIMER0A-16);             // turn-on interrupt 37 (TIMER1A)
}

void startTimer0()
{
    TIMER0_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
}

void stopTimer0()
{
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;                  // turn-on timer
}

void setTimerLoadValue(uint32_t loadValue)
{
    TIMER0_TAILR_R = loadValue;
}

int main(void)
{
    initTimer0();
    initUart0();
    TRIGGER = 0;
    int start = 0;
    int end = 0;
    char out[50];

    sprintf(out,"New one\n");
    putsUart0(out);

    while(1)
    {
        TIMER0_TAV_R = 0;
        TRIGGER = 1;
        waitMicrosecond(15);
        TRIGGER = 0;
        while(!ECHO);
        startTimer0();
        while(ECHO == 1)
        {
            end = TIMER0_TAV_R;
        }
        stopTimer0();
        double time = (((double)(end - start) * 0.025)*0.001);
        double distance = (time * 340)/20;
//        int distance = 340*timeReal;

        sprintf(out,"distance = %.3f\n", distance);
        putsUart0(out);
        waitMicrosecond(5000000);
    }

}

