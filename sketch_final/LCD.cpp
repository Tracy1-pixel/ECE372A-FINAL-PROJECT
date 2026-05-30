#include "LCD.h"

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdint.h>

// Arduino Mega 2560 pin mapping:
// Pin 22 = PA0 = RS
// Pin 23 = PA1 = E
// Pin 24 = PA2 = D4
// Pin 25 = PA3 = D5
// Pin 26 = PA4 = D6
// Pin 27 = PA5 = D7

#define LCD_PORT PORTA
#define LCD_DDR  DDRA

#define LCD_RS PA0
#define LCD_E  PA1
#define LCD_D4 PA2
#define LCD_D5 PA3
#define LCD_D6 PA4
#define LCD_D7 PA5

/*
 * Software delay functions for LCD timing.
 */
static void lcdDelayCycles(uint16_t count) {
    for (volatile uint16_t i = 0; i < count; i++) {
        __asm__ __volatile__("nop");
    }
}

static void lcdDelayUs(uint16_t us) {
    while (us--) {
        lcdDelayCycles(16);
    }
}

static void lcdDelayMs(uint16_t ms) {
    while (ms--) {
        lcdDelayUs(1000);
    }
}

static void pulseEnable() {
    LCD_PORT |= (1 << LCD_E);    // Set Enable pin HIGH.
    lcdDelayUs(1);               // Short delay so LCD can detect the pulse.

    LCD_PORT &= ~(1 << LCD_E);   // Set Enable pin LOW.
    lcdDelayUs(100);             // Wait for LCD to process the data.
}

static void sendNibble(uint8_t nibble) {
    // Clear LCD data pins D4-D7 before sending new data.
    LCD_PORT &= ~((1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7));

    // Send each bit of the 4-bit nibble to LCD D4-D7.
    if (nibble & 0x01) LCD_PORT |= (1 << LCD_D4);
    if (nibble & 0x02) LCD_PORT |= (1 << LCD_D5);
    if (nibble & 0x04) LCD_PORT |= (1 << LCD_D6);
    if (nibble & 0x08) LCD_PORT |= (1 << LCD_D7);

    pulseEnable();  // Latch the nibble into the LCD.
}

static void sendByte(uint8_t data, uint8_t rs) {
    // RS = 0 means command, RS = 1 means display data.
    if (rs) {
        LCD_PORT |= (1 << LCD_RS);
    } else {
        LCD_PORT &= ~(1 << LCD_RS);
    }

    // Send high 4 bits first, then low 4 bits.
    sendNibble(data >> 4);
    sendNibble(data & 0x0F);

    lcdDelayUs(50);  // Wait for LCD to process the byte.
}

static void command(uint8_t cmd) {
    sendByte(cmd, 0);  // Send a command byte to LCD.
}

void writeCharacter(unsigned char c) {
    sendByte(c, 1);  // Send one display character to LCD.
}

void writeString(const char* str) {
    // Write characters until the null terminator '\0' is reached.
    while (*str) {
        writeCharacter(*str);
        str++;
    }
}

void initLCD() {
    LCD_DDR |= (1 << LCD_RS) |
               (1 << LCD_E)  |
               (1 << LCD_D4) |
               (1 << LCD_D5) |
               (1 << LCD_D6) |
               (1 << LCD_D7);

    LCD_PORT &= ~((1 << LCD_RS) |
                  (1 << LCD_E)  |
                  (1 << LCD_D4) |
                  (1 << LCD_D5) |
                  (1 << LCD_D6) |
                  (1 << LCD_D7));

    lcdDelayMs(50);

    // 4-bit initialization sequence from HD44780 datasheet
    sendNibble(0x03);
    lcdDelayMs(5);

    sendNibble(0x03);
    lcdDelayMs(5);

    sendNibble(0x03);
    lcdDelayMs(1);

    sendNibble(0x02);
    lcdDelayMs(1);

    command(0x28);   // 4-bit, 2-line, 5x7
    lcdDelayMs(1);

    command(0x08);   // display off
    lcdDelayMs(1);

    command(0x01);   // clear
    lcdDelayMs(3);

    command(0x06);   // entry mode
    lcdDelayMs(1);

    command(0x0C);   // display on, cursor off
    lcdDelayMs(1);
}

// Move the LCD cursor to a specific position.
// x = column number
// y = row number, where 0 is the first row and 1 is the second row
void moveCursor(unsigned char x, unsigned char y) {
    uint8_t address;

    // The first LCD row starts at DDRAM address 0x00.
    if (y == 0) {
        address = 0x00 + x;
    }
    // The second LCD row starts at DDRAM address 0x40.
    else {
        address = 0x40 + x;
    }

    // 0x80 is the HD44780 command for setting the DDRAM cursor address.
    // OR-ing with address creates the final cursor-position command.
    command(0x80 | address);
}

// Write a two-digit number to the LCD.
// For example, value = 7 will display "07", and value = 25 will display "25".
static void writeTwoDigits(uint8_t value) {
    // Write the tens digit.
    writeCharacter((value / 10) + '0');

    // Write the ones digit.
    writeCharacter((value % 10) + '0');
}

// Display the current clock time on the first row of the LCD.
// Format: HH:MM:SS
void showClock(uint8_t hour, uint8_t minute, uint8_t second) {
    // Move to the first column of the first row.
    moveCursor(0, 0);

    // Display hour.
    writeTwoDigits(hour);

    // Display time separator.
    writeCharacter(':');

    // Display minute.
    writeTwoDigits(minute);

    // Display time separator.
    writeCharacter(':');

    // Display second.
    writeTwoDigits(second);
}

// Display the countdown status and remaining time on the second row of the LCD.
// Example output: "R 02:30 " if mode begins with 'R'.
void showCountdown(const char* mode, uint8_t minutes, uint8_t seconds) {
    // Move to the first column of the second row.
    moveCursor(0, 1);

    // Display the first character of the mode string.
    // For example, "RUN" displays 'R', "STOP" displays 'S', and "WAIT" displays 'W'.
    writeCharacter(mode[0]);

    // Add a space between the mode letter and the countdown time.
    writeCharacter(' ');

    // Display countdown minutes as two digits.
    writeCharacter((minutes / 10) + '0');
    writeCharacter((minutes % 10) + '0');

    // Display time separator.
    writeCharacter(':');

    // Display countdown seconds as two digits.
    writeCharacter((seconds / 10) + '0');
    writeCharacter((seconds % 10) + '0');

    // Clear the last unused LCD character to remove old leftover text.
    writeCharacter(' ');
}