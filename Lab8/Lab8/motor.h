#ifndef PEDOMETER_H
#define PEDOMETER_H

#define SLEEP (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 3*4)))

// PortB masks
#define RED_BL_LED_MASK 32
#define ORANGE_BL_LED_MASK 16
#define SLEEP_MASK 8

// PortE masks
#define BLUE_BL_LED_MASK 16
#define GREEN_BL_LED_MASK 32

void initMotor();
void setPwmDutyCycle(uint8_t id, uint16_t pwmA, uint16_t pwmB);
void waitMicrosecond(uint32_t us);

void moveForward(uint16_t distance_cm);
void reverse(uint16_t distance_cm);

void rotateCW(uint16_t degrees);
void rotateCCW(uint16_t degrees);


#endif
