#include <stdint.h>
#include <stdbool.h>
#include "sensor.h"
#include "motor.h"
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

#define TRIGGER (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4)))
#define ECHO    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4)))

#define TRIGGER_MASK 8
#define ECHO_MASK 4

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
    TIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;          // configure for onehsot mode (count up)
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

int calculateStopDistance()
{
    TRIGGER = 0;
    int start = 0;
    int end = 0;

    TIMER0_TAV_R = 0;
    TRIGGER = 1;
    waitMicrosecond(13);
    TRIGGER = 0;
    while(!ECHO);
    startTimer0();
    while(ECHO == 1)
    {
        end = TIMER0_TAV_R;
    }
    stopTimer0();
    double time = (((double)(end - start) * 0.025)*0.001);
    double doubleDistance = (time * (340 - 143))/20;
    int distance = (int)doubleDistance;
    waitMicrosecond(60000);

    return distance;
}

//int stopFrom(int StopDistance)
//{
//    initTimer0();
//    int distance = calculateStopDistance();
//
//    while(distance > StopDistance)
//    {
//        distance = calculateStopDistance();
//
//        setPwmDutyCycle(1, 0, 90);
//        setPwmDutyCycle(0, 90, 0);
//    }
//    setPwmDutyCycle(1, 0, 0);
//    setPwmDutyCycle(0, 0, 0);
//
//}

