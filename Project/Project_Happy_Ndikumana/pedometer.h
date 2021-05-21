
#ifndef PEDOMETER_H
#define PEDOMETER_H

// PortC masks
#define FREQ_IN_MASK0 64
#define FREQ_IN_MASK1 16

#define SLEEP (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 3*4)))

void initPedometer();
int countBalance(uint8_t id, int count0, int count1);
void setTimerModeUp();
void setTimerModeDown();
void setEncoderPosition(uint8_t id, int32_t position);
int32_t getEncoderPosition(uint8_t id);
void selectEncoderIncMode(uint8_t id);
void selectEncoderDecMode(uint8_t id);

#endif
