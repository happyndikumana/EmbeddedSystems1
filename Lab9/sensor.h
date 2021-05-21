#ifndef SENSOR_H_
#define SENSOR_H_

#define TRIGGER (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4)))
#define ECHO    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4)))

#define TRIGGER_MASK 8
#define ECHO_MASK 4

void initTimer0();
void startTimer0();
void stopTimer0();
void setTimerLoadValue(uint32_t loadValue);
int stopFrom(int distance);

#endif
