#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern volatile uint8_t g_1sFlag;
extern volatile uint8_t g_100msFlag;

void initTimer0();

#endif