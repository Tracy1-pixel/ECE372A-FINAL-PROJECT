#ifndef I2C_H
#define I2C_H

#include <stdint.h>

class I2C {
public:
    static void init();
    static void start();
    static void stop();
    static void write(uint8_t data);
    static uint8_t readAck();
    static uint8_t readNack();
};

#endif