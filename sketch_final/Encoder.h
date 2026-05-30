#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

class Encoder {
private:
    uint8_t lastCLK;
    uint8_t lastButton;

public:
    Encoder();

    void init();
    int8_t readRotation();
    uint8_t isPressed();
};

#endif