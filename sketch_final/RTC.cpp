#include "RTC.h"
#include <avr/io.h>

// Arduino Mega 2560 pin mapping:
// Digital Pin 30 = PC7
// Digital Pin 31 = PC6
// Digital Pin 32 = PC5
// 3-wire serial bit-banging

#define RTC_CLK PC7
#define RTC_DAT PC6
#define RTC_RST PC5

#define RTC_PORT PORTC
#define RTC_PIN  PINC
#define RTC_DDR  DDRC

/*
 * Small software delay for RTC bit-banging.
 * Exact timing is not critical here; it only gives CLK and DAT signals
 * a short stable time between transitions.
 */
static void rtcTinyDelay() {
    for (volatile uint8_t i = 0; i < 32; i++) {
        __asm__ __volatile__("nop");
    }
}

RTC::RTC() {
    // Initialize stored time values
    hour = 0;
    minute = 0;
    second = 0;
}

uint8_t RTC::bcdToDec(uint8_t value) {
    // Convert BCD format to decimal format
    return ((value >> 4) * 10) + (value & 0x0F);
}

uint8_t RTC::decToBcd(uint8_t value) {
    // Convert decimal format to BCD format
    return ((value / 10) << 4) | (value % 10);
}

// Set CLK pin HIGH
static void clkHigh() {
    RTC_PORT |= (1 << RTC_CLK);
}

// Set CLK pin LOW
static void clkLow() {
    RTC_PORT &= ~(1 << RTC_CLK);
}

// Set RST pin HIGH to start communication
static void rstHigh() {
    RTC_PORT |= (1 << RTC_RST);
}

// Set RST pin LOW to end communication
static void rstLow() {
    RTC_PORT &= ~(1 << RTC_RST);
}

// Set DAT pin HIGH to send logic 1
static void datHigh() {
    RTC_PORT |= (1 << RTC_DAT);
}

// Set DAT pin LOW to send logic 0
static void datLow() {
    RTC_PORT &= ~(1 << RTC_DAT);
}

// Configure DAT pin as output when writing data to RTC
static void datOutput() {
    RTC_DDR |= (1 << RTC_DAT);
}

// Configure DAT pin as input when reading data from RTC
static void datInput() {
    RTC_DDR &= ~(1 << RTC_DAT);
}

// Read the current logic level on DAT pin
static uint8_t datRead() {
    return (RTC_PIN & (1 << RTC_DAT)) ? 1 : 0;
}

void RTC::init() {
    // Configure CLK, RST, and DAT as output pins
    RTC_DDR |= (1 << RTC_CLK);
    RTC_DDR |= (1 << RTC_RST);
    RTC_DDR |= (1 << RTC_DAT);

    // Set all communication pins to default LOW state
    clkLow();
    rstLow();
    datLow();
}

void RTC::writeByte(uint8_t data) {
    // RTC receives data from MCU, so DAT must be output
    datOutput();

    // Send 8 bits, LSB first
    for (uint8_t i = 0; i < 8; i++) {
        // Output the lowest bit of data
        if (data & 0x01) {
            datHigh();
        } else {
            datLow();
        }

        // Generate one clock pulse so RTC can read the bit
        clkHigh();
        rtcTinyDelay();

        clkLow();
        rtcTinyDelay();

        // Move the next bit into bit 0 position
        data >>= 1;
    }
}

uint8_t RTC::readByte() {
    uint8_t data = 0;

    // RTC sends data to MCU, so DAT must be input
    datInput();

    // Read 8 bits, LSB first
    for (uint8_t i = 0; i < 8; i++) {
        // If DAT is HIGH, store 1 in the current bit position
        if (datRead()) {
            data |= (1 << i);
        }

        // Generate one clock pulse to read the next bit
        clkHigh();
        rtcTinyDelay();

        clkLow();
        rtcTinyDelay();
    }

    return data;
}

void RTC::writeRegister(uint8_t address, uint8_t data) {
    // Start RTC communication
    rstHigh();
    rtcTinyDelay();

    // Send register address first
    writeByte(address);

    // Then send data to be written into that register
    writeByte(data);

    // End RTC communication
    rstLow();
}

uint8_t RTC::readRegister(uint8_t address) {
    uint8_t data;

    // Start RTC communication
    rstHigh();
    rtcTinyDelay();

    // Send the register address to read from RTC
    writeByte(address);

    // Read one byte returned by RTC
    data = readByte();

    // End RTC communication
    rstLow();

    return data;
}

void RTC::readTime() {
    // Read seconds register.
    // 0x81 is the seconds read address.
    second = bcdToDec(readRegister(0x81) & 0x7F);

    // Read minutes register.
    // 0x83 is the minutes read address.
    minute = bcdToDec(readRegister(0x83) & 0x7F);

    // Read hours register.
    // 0x85 is the hours read address.
    hour = bcdToDec(readRegister(0x85) & 0x3F);
}

void RTC::setTime(uint8_t h, uint8_t m, uint8_t s) {
    // Disable write protection before writing time registers
    writeRegister(0x8E, 0x00);

    // Write seconds, minutes, and hours in BCD format
    writeRegister(0x80, decToBcd(s));
    writeRegister(0x82, decToBcd(m));
    writeRegister(0x84, decToBcd(h));

    // Enable write protection again after setting time
    writeRegister(0x8E, 0x80);
}

uint8_t RTC::getHour() {
    // Return the most recently read hour value
    return hour;
}

uint8_t RTC::getMinute() {
    // Return the most recently read minute value
    return minute;
}

uint8_t RTC::getSecond() {
    // Return the most recently read second value
    return second;
}