#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timer.h"

volatile uint8_t g_1sFlag = 0;
volatile uint8_t g_100msFlag = 0;

static volatile uint16_t msCount = 0;

void initTimer0() {//set CTC mode
    TCCR0A = (1 << WGM01);
    TCCR0B = 0;

    OCR0A = 249;

    TCCR0B |= (1 << CS01) | (1 << CS00);
    TIMSK0 |= (1 << OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
    msCount++;

    if (msCount % 100 == 0) {
        g_100msFlag = 1;
    }//seting flag of 1ms to do task 

    if (msCount >= 1000) {
        g_1sFlag = 1;
        msCount = 0;
    }//setting flag of 1s to do task
}