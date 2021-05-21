
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD Interface
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red Backlight LED:
//   M0PWM3 (PB5) drives an NPN transistor that powers the red LED
// Green Backlight LED:
//   M0PWM5 (PE5) drives an NPN transistor that powers the green LED
// Blue Backlight LED:
//   M0PWM4 (PE4) drives an NPN transistor that powers the blue LED
// Blue Backlight LED:
//   M0PWM4 (PB4) drives an NPN transistor that powers the blue LED

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
//#include "uart0.h"
#include "motor.h"

//#define SLEEP      (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 3*4)))
//
//// PortB masks
//#define RED_BL_LED_MASK 32
//#define ORANGE_BL_LED_MASK 16
//#define SLEEP_MASK 8
//
//// PortE masks
//#define BLUE_BL_LED_MASK 16
//#define GREEN_BL_LED_MASK 32

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

const int wheel1CDuty[6] = {18, 27, 28, 22, 23, 24}; //left wheel
const int wheel0CDuty[6] = {18, 29, 30, 23, 25, 27}; //right wheel
const int dutyCycles[6] = {95, 90, 85, 80, 75, 70};

//int count0;
//int count1;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Motors
void initMotor()
{
    // Enable clocks
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1 | SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);

    // Configure four GPO
    GPIO_PORTB_DIR_R |= RED_BL_LED_MASK | ORANGE_BL_LED_MASK | SLEEP_MASK;                       // make bit5 an output
    GPIO_PORTB_DR2R_R |= RED_BL_LED_MASK | ORANGE_BL_LED_MASK | SLEEP_MASK;                      // set drive strength to 2mA
    GPIO_PORTB_DEN_R |= RED_BL_LED_MASK | ORANGE_BL_LED_MASK | SLEEP_MASK;                       // enable digital
    GPIO_PORTB_AFSEL_R |= RED_BL_LED_MASK | ORANGE_BL_LED_MASK;                     // select auxilary function
    GPIO_PORTB_PCTL_R &= ~(GPIO_PCTL_PB5_M | GPIO_PCTL_PB4_M);                      // enable PWM
    GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB5_M0PWM3 | GPIO_PCTL_PB4_M0PWM2;
    GPIO_PORTE_DIR_R |= GREEN_BL_LED_MASK | BLUE_BL_LED_MASK;                       // make bits 4 and 5 outputs
    GPIO_PORTE_DR2R_R |= GREEN_BL_LED_MASK | BLUE_BL_LED_MASK;                      // set drive strength to 2mA
    GPIO_PORTE_DEN_R |= GREEN_BL_LED_MASK | BLUE_BL_LED_MASK;                       // enable digital
    GPIO_PORTE_AFSEL_R |= GREEN_BL_LED_MASK | BLUE_BL_LED_MASK;                     // select auxilary function
    GPIO_PORTE_PCTL_R &= ~(GPIO_PCTL_PE4_M | GPIO_PCTL_PE5_M);                      // enable PWM
    GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_M0PWM4 | GPIO_PCTL_PE5_M0PWM5;

    // Configure PWM module 0 to drive RGB backlight
    // ORANG on M0PWM2 (PB4), MOPWM1a
    // RED   on M0PWM3 (PB5), M0PWM1b
    // BLUE  on M0PWM4 (PE4), M0PWM2a
    // GREEN on M0PWM5 (PE5), M0PWM2b
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;                // reset PWM0 module
    SYSCTL_SRPWM_R = 0;                              // leave reset state
    PWM0_1_CTL_R = 0;                                // turn-off PWM0 generator 1 (drives outs 2 and 3)
    PWM0_2_CTL_R = 0;                                // turn-off PWM0 generator 2 (drives outs 4 and 5)
    PWM0_1_GENB_R = PWM_0_GENB_ACTCMPBD_ZERO | PWM_0_GENB_ACTLOAD_ONE;
                                                     // output 3 on PWM0, gen 1b, cmpb
    PWM0_2_GENA_R = PWM_0_GENA_ACTCMPAD_ZERO | PWM_0_GENA_ACTLOAD_ONE;
                                                     // output 4 on PWM0, gen 2a, cmpa
    PWM0_2_GENB_R = PWM_0_GENB_ACTCMPBD_ZERO | PWM_0_GENB_ACTLOAD_ONE;
                                                     // output 5 on PWM0, gen 2b, cmpb
    PWM0_1_GENA_R = PWM_0_GENA_ACTCMPAD_ZERO | PWM_0_GENA_ACTLOAD_ONE;
                                                     // output 2 on PWM0, gen 1a, cmpb
    PWM0_1_LOAD_R = 1024;                            // set frequency to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz
    PWM0_2_LOAD_R = 1024;
    PWM0_INVERT_R = PWM_INVERT_PWM3INV | PWM_INVERT_PWM4INV | PWM_INVERT_PWM5INV | PWM_INVERT_PWM2INV;
                                                     // invert outputs so duty cycle increases with increasing compare values
    PWM0_1_CMPB_R = 512;                               // red off (0=always low, 1023=always high)
    PWM0_1_CMPA_R = 512;                               //
    PWM0_2_CMPB_R = 512;                               // green off
    PWM0_2_CMPA_R = 512;                               // blue off

    PWM0_1_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM0 generator 1
    PWM0_2_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM0 generator 2
    PWM0_ENABLE_R = PWM_ENABLE_PWM3EN | PWM_ENABLE_PWM4EN | PWM_ENABLE_PWM5EN | PWM_ENABLE_PWM2EN;
                                                     // enable outputs
}

void setPwmDutyCycle(uint8_t id, uint16_t pwmA, uint16_t pwmB)
{
    if(id == 1)
    {
        PWM0_1_CMPA_R = (pwmA * 1024) / 100;
        PWM0_1_CMPB_R = (pwmB * 1024) / 100;
    }
    else if(id == 0)
    {
        PWM0_2_CMPA_R = (pwmA * 1024) / 100;
        PWM0_2_CMPB_R = (pwmB * 1024) / 100;
    }
}

void balance2(int difference, int idealDifference, int count0, int count1, int idealCounts, int dutyCycle, int direction)
{
    if(difference == idealDifference)
        return;

    if((difference > idealDifference) && (count0 < idealCounts && count1 < idealCounts)) //wheel 1 is ahead;
    {
        while((difference > idealDifference) && (count0 < idealCounts && count1 < idealCounts))
        {
            if(direction == 1) //forward
            {

                setPwmDutyCycle(1, 0, dutyCycle);
                setPwmDutyCycle(0, 75, 0);
            }
            else if (direction == 0)
            {

                setPwmDutyCycle(1, dutyCycle, 0);
                setPwmDutyCycle(0, 0, 75);
            }
            else
            {
                if(direction == 2) //CW rotation;
                {
                    setPwmDutyCycle(1, 0, 70);
                    setPwmDutyCycle(0, 0, dutyCycle);
                }
                else if (direction == 3)//CCW rotation
                {
                    setPwmDutyCycle(1, 70, 0);
                    setPwmDutyCycle(0, dutyCycle, 0);
                }
            }
            count0 = getEncoderPosition(0);
            count1 = getEncoderPosition(1);
            difference = count0 - count1;
        }
    }
    if((difference < idealDifference) && (count0 < idealCounts && count1 < idealCounts)) //Wheel 0 is ahead
    {
        while((difference < idealDifference) && (count0 < idealCounts && count1 < idealCounts))
        {
            if(direction == 1) //forward
            {

                setPwmDutyCycle(1, 0, 75);
                setPwmDutyCycle(0, dutyCycle, 0);
            }
            else if (direction == 0) //backwards
            {

                setPwmDutyCycle(1, 75, 0);
                setPwmDutyCycle(0, 0, dutyCycle);
            }
            else
            {
                if(direction == 2) //CW rotation;
                {
                    setPwmDutyCycle(1, 0, dutyCycle);
                    setPwmDutyCycle(0, 0, 70);
                }
                else if (direction == 3)//CCW rotation
                {
                    setPwmDutyCycle(1, dutyCycle, 0);
                    setPwmDutyCycle(0, 70, 0);
                }
            }
            count0 = getEncoderPosition(0);
            count1 = getEncoderPosition(1);
            difference = count0 - count1;
        }
    }

    switch(direction)
    {
        case 0:
        {
            setPwmDutyCycle(1, dutyCycle, 0);
            setPwmDutyCycle(0, 0, dutyCycle);
        }
        case 1:
        {
            setPwmDutyCycle(1, 0, dutyCycle);
            setPwmDutyCycle(0, dutyCycle, 0);
        }
        case 2:
        {
            setPwmDutyCycle(0, 0, dutyCycle);
            setPwmDutyCycle(1, 0, dutyCycle);
        }
        case 3:
        {
            setPwmDutyCycle(0, dutyCycle,0);
            setPwmDutyCycle(1, dutyCycle,0);
        }
        default:
        {
            return;
        }
    }
//    if(direction == 0 || direction == 1)
//    setPwmDutyCycle(1, dutyCycle, 0);
//    setPwmDutyCycle(0, 0, dutyCycle);
}
double distance(double distance)
{
    double distanceBack = (double)(distance * (1.0/0.4982));
    return distanceBack;
}

void forward(uint16_t distance_cm)
{
    //set the counts to 0
    //calculate counts for given distance
    //loop until calculated count is reached.
    //take current count and add calculated count. Robot will stop when the new total is reached.

    //counts = (distance_cm/20.34495) * 21;

    char out[50];
    int idealDifference, i;
    int dutyCycle = 90;

    for(i = 0; i < 6; i++)
        {
            if(dutyCycles[i] == dutyCycle)
            {
                idealDifference = wheel1CDuty[i] - wheel0CDuty[i];
                break;
            }
        }


    setEncoderPosition(1, 0);
    setEncoderPosition(0, 0);
    double falseDistance = (double)distance_cm;
    double counts = (distance(falseDistance)/ 20.344) * wheel0CDuty[1];
    int idealCounts = (int)counts;
    int count0 = wheel0CDuty[1];
    int count1 = wheel1CDuty[1];

    while(count0 < idealCounts - 1 && count1 < idealCounts - 1)
    {
        setPwmDutyCycle(1, 0, dutyCycle);
        setPwmDutyCycle(0, dutyCycle, 0);


        count0 = getEncoderPosition(0);
        count1 = getEncoderPosition(1);
        int difference = count0 - count1;

        if(difference != idealDifference)
        {
            balance2(difference, idealDifference, count0, count1, idealCounts, dutyCycle, 1);
        }
            //balance(90, 0, idealCounts, count0, count1);
    }
    sprintf(out,"C0 = %d, C1 = %d, ideal = %d. outside loop\n", count0, count1, idealCounts);
    putsUart0(out);
    SLEEP = 0;
}
void reverse(uint16_t distance_cm)
{
    //set the counts to 0
    //calculate counts for given distance
    //loop until calculated count is reached.
    //take current count and add calculated count. Robot will stop when the new total is reached.

    //counts = (distance_cm/20.34495) * 21;

    char out[50];
    int idealDifference, i;
    int dutyCycle = 90;

    for(i = 0; i < 6; i++)
        {
            if(dutyCycles[i] == dutyCycle)
            {
                idealDifference = wheel1CDuty[i] - wheel0CDuty[i];
                break;
            }
        }

    setEncoderPosition(1, 0);
    setEncoderPosition(0, 0);

    double falseDistance = (double)distance_cm;
    double counts = (distance(falseDistance)/ 20.344) * wheel0CDuty[1];
    int idealCounts = (int)counts;
    int count0 = wheel0CDuty[1];
    int count1 = wheel1CDuty[1];

    while(count0 < idealCounts - 1 && count1 < idealCounts - 1)
    {
        setPwmDutyCycle(1, dutyCycle, 0);
        setPwmDutyCycle(0, 0, dutyCycle);

//        setPwmDutyCycle(1, 95, 0); //right
//        setPwmDutyCycle(0, 0, 95); //left

        count0 = getEncoderPosition(0);
        count1 = getEncoderPosition(1);
        int difference = count0 - count1;

        if(difference != idealDifference)
        {
            balance2(difference, idealDifference, count0, count1, idealCounts, dutyCycle, 0);
        }
            //balance(90, 0, idealCounts, count0, count1);
    }
    sprintf(out,"C0 = %d, C1 = %d, ideal = %d. outside loop\n", count0, count1, idealCounts);
    putsUart0(out);
    SLEEP = 0;
}

void rotateCW(uint16_t degrees)
{
    //Car width (diameter)= 18.7 cm =>
    //circumference = 2 * pi * (diamter/2) => C = 58.7478 cm
    //1 degree = C/360 => 58.7478/360 = 0.163188 cm

    int idealDifference, i;
    int dutyCycle = 90;

    for(i = 0; i < 6; i++)
        {
            if(dutyCycles[i] == dutyCycle)
            {
                idealDifference = wheel1CDuty[i] - wheel0CDuty[i];
                break;
            }
        }

    setEncoderPosition(1, 0);
    setEncoderPosition(0, 0);

    double realDegrees = (double)degrees * (1/2.2221);
    double degreeDistanceConst = distance(0.163188); // cm/degree

    double distance = realDegrees * degreeDistanceConst;

    char out[50];
    double counts = (distance / 20.344) * wheel0CDuty[1];
    int idealCounts = (int)counts;
    int count0 = 0;
    int count1 = 0;

    while(count0 < idealCounts - 1 && count1 < idealCounts - 1)
    {
        setPwmDutyCycle(0, 0, dutyCycle);
        setPwmDutyCycle(1, 0, dutyCycle);

        count0 = getEncoderPosition(0);
        count1 = getEncoderPosition(1);
        count1 = countBalance(1, count0, count1);

        int difference = count0 - count1;
//
//        if(difference != idealDifference)
//        {
//            balance2(difference, idealDifference, count0, count1, idealCounts, dutyCycle, 2);
//        }
    }
    sprintf(out,"C0 = %d, C1 = %d, ideal = %d. outside loop\n", count0, count1, idealCounts);
    putsUart0(out);
    SLEEP = 0;
}

void rotateCCW(uint16_t degrees)
{
    //Car width (diameter)= 18.7 cm =>
    //circumference = 2 * pi * (diamter/2) => C = 58.7478 cm
    //1 degree = C/360 => 58.7478/360 = 0.163188 cm

    int idealDifference, i;
    int dutyCycle = 90;

    for(i = 0; i < 6; i++)
        {
            if(dutyCycles[i] == dutyCycle)
            {
                idealDifference = wheel1CDuty[i] - wheel0CDuty[i];
                break;
            }
        }

    setEncoderPosition(1, 0);
    setEncoderPosition(0, 0);

    double realDegrees = (double)degrees * (1/2.1221);
    double degreeDistanceConst = distance(0.163188); // cm/degree

    double distance = realDegrees * degreeDistanceConst;

    char out[50];
    double counts = (distance / 20.344) * wheel0CDuty[1];
    int idealCounts = (int)counts;
    int count0 = 0;
    int count1 = 0;

    while(count0 < idealCounts - 1 && count1 < idealCounts - 1)
    {
        setPwmDutyCycle(0, dutyCycle, 0);
        setPwmDutyCycle(1, dutyCycle, 0);

        count0 = getEncoderPosition(0);
        count1 = getEncoderPosition(1);
        count1 = countBalance(1, count0, count1);

        int difference = count0 - count1;

//        if(difference != idealDifference)
//        {
//            balance2(difference, idealDifference, count0, count1, idealCounts, dutyCycle, 3);
//        }
    }
    sprintf(out,"C0 = %d, C1 = %d, ideal = %d. outside loop\n", count0, count1, idealCounts);
    putsUart0(out);
    SLEEP = 0;
}
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
