#include "Encoder.h"
#include <avr/io.h>

// Mega2560 pin mapping:
// Pin 18 = PD3 = CLK
// Pin 19 = PD2 = DT
// Pin 2  = PE4 = SW

#define ENC_CLK PD3
#define ENC_DT  PD2
#define ENC_SW  PE4

Encoder::Encoder() {
    lastCLK = 1;
    lastButton = 1;
}

void Encoder::init() {
    // CLK and DT input
    DDRD &= ~((1 << ENC_CLK) | (1 << ENC_DT));

    // enable pull-up on CLK and DT
    PORTD |= (1 << ENC_CLK) | (1 << ENC_DT);

    // SW input
    DDRE &= ~(1 << ENC_SW);

    // enable pull-up on SW
    PORTE |= (1 << ENC_SW);

    lastCLK = (PIND & (1 << ENC_CLK)) ? 1 : 0;
    lastButton = (PINE & (1 << ENC_SW)) ? 1 : 0;
}

int8_t Encoder::readRotation() {
    uint8_t currentCLK = (PIND & (1 << ENC_CLK)) ? 1 : 0;
    int8_t direction = 0;

    // Detect falling edge
    if (lastCLK == 1 && currentCLK == 0) {
        if (PIND & (1 << ENC_DT)) {
            direction = 1;    // clockwise
        } else {
            direction = -1;   // counter-clockwise
        }
    }

    lastCLK = currentCLK;
    return direction;
}

uint8_t Encoder::isPressed() {
    return !(PINE & (1 << ENC_SW));  // pressed = LOW
}