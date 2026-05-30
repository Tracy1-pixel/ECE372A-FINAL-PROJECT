#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void initLCD();
void moveCursor(unsigned char x, unsigned char y);
void writeCharacter(unsigned char c);
void writeString(const char* str);

void showClock(uint8_t hour, uint8_t minute, uint8_t second);
void showCountdown(const char* mode, uint8_t minutes, uint8_t seconds);

#endif