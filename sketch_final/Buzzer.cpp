#include "Buzzer.h"
#include <avr/io.h>

// Mega2560 Digital Pin 9 = PH6

#define BUZZER_DDR  DDRH
#define BUZZER_PORT PORTH
#define BUZZER_PIN  PH6

Buzzer::Buzzer() {
    patternStep = 0;
}

void Buzzer::init() {
    BUZZER_DDR |= (1 << BUZZER_PIN);
    off();
}

void Buzzer::on() {
    BUZZER_PORT |= (1 << BUZZER_PIN);
}

void Buzzer::off() {
    BUZZER_PORT &= ~(1 << BUZZER_PIN);
}

void Buzzer::updatePattern() {
    switch (patternStep) {
    case 0:
        on();      // beep 1
        break;

    case 1:
        off();     // short gap
        break;

    case 2:
        on();      // beep 2
        break;

    case 3:
    case 4:
    case 5:
        off();     // longer pause
        break;
    }

    patternStep++;

    if (patternStep >= 6) {
        patternStep = 0;//loop
    }
}