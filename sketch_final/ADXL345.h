#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

enum Orientation {
    ORIENT_UNKNOWN,
    ORIENT_LYING,
    ORIENT_STANDING
};

class ADXL345 {
private:
    int16_t x;
    int16_t y;
    int16_t z;

    void writeRegister(uint8_t reg, uint8_t value);

public:
    ADXL345();

    void init();
    void readAccel();

    Orientation getOrientation();

    int16_t getX();
    int16_t getY();
    int16_t getZ();
};

#endif