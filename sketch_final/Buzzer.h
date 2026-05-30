#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

class Buzzer {
private:
    uint8_t patternStep;

public:
    Buzzer();

    void init();
    void on();
    void off();

    void updatePattern();
};

#endif