#ifndef RTC_H
#define RTC_H

#include <stdint.h>

class RTC {
private:
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint8_t bcdToDec(uint8_t value);
    uint8_t decToBcd(uint8_t value);

    void writeByte(uint8_t data);
    uint8_t readByte();

    void writeRegister(uint8_t address, uint8_t data);
    uint8_t readRegister(uint8_t address);

public:
    RTC();

    void init();
    void readTime();

    void setTime(uint8_t h, uint8_t m, uint8_t s);

    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
};

#endif