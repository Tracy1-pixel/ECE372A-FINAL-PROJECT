# ECE372A-FINAL-PROJECT
# Smart Flippable Alarm Clock

## Project Overview

This project is a Smart Flippable Pomodoro Timer designed for ECE 372A: Microcontroller Organization. The goal of the project is to create a portable study timer that combines real-time clock functionality, countdown timing, motion-based interaction, user input, and an audible alarm system.

Many students struggle with staying focused while studying and managing work and break periods effectively. Traditional timers require manual button input and may continue running even when the user is no longer actively studying. This project solves that problem by using a motion-sensitive flip interaction to make the timer more convenient and engaging.

## Team Members

- Rigel Abcede — Enclosure Design, Rotary Encoder, I2C
- Changyuan Chen — RTC, ADXL345, Main Controller Programming

## Features

- Real-time clock display using a DS1302 RTC module
- Countdown timer for study or focus sessions
- Flip-based interaction using an ADXL345 accelerometer
- User input through a KY-040 rotary encoder
- 16x2 LCD display for clock, timer, and menu information
- Buzzer alarm when the countdown finishes
- Compact 3D-printed enclosure

## System Requirements

### Accurate Timekeeping

The system displays real-time clock information using the RTC module.

Target measurement:

- Time drift ≤ ±1 minute over 24 hours

### Reliable Flip Detection

The ADXL345 accelerometer detects orientation changes.

Target measurement:

- At least 95% correct detection over 20 flips

### Countdown Accuracy

The countdown timer tracks time accurately.

Target measurement:

- Error ≤ ±1 second per minute

### User Input Responsiveness

The rotary encoder responds quickly and correctly.

Target measurement:

- Response delay ≤ 200 ms
- Correct input detection ≥ 95%

## Hardware Components

### Input Devices

- KY-040 Rotary Encoder
- ADXL345 Accelerometer

### Output Devices

- 16x2 LCD Display
- Buzzer

### Communication Protocols

- I2C for ADXL345 accelerometer
- 3-wire communication for DS1302 RTC module
- CLK and DT signals for KY-040 rotary encoder

## Main Components

### Arduino Mega 2560

The Arduino Mega 2560 is used as the main microcontroller. It controls the LCD, RTC, accelerometer, rotary encoder, buzzer, and timer logic.

### ADXL345 Accelerometer

The ADXL345 is a 3-axis accelerometer used to detect the physical orientation of the device. It communicates with the microcontroller using I2C.

Main functions:

- Detects whether the device is flat or flipped
- Enables motion-based user interaction
- Helps make the timer more interactive and convenient

### DS1302 Real-Time Clock Module

The DS1302 RTC module keeps track of real-world time. It communicates with the microcontroller using 3-wire communication.

Main functions:

- Tracks seconds, minutes, hours, day, date, month, and year
- Maintains time using battery backup when main power is off
- Provides real-time clock data for the LCD display

### KY-040 Rotary Encoder

The KY-040 rotary encoder is used for menu navigation and timer setting.

Main functions:

- Detects clockwise and counterclockwise rotation
- Allows users to adjust timer values
- Provides push-button input for selection

### 16x2 LCD Display

The LCD displays the real-time clock, countdown timer, menu options, and system status.

### Buzzer

The buzzer alerts the user when the countdown timer finishes.

## Software Structure

The project software is divided into multiple source files for modular design.

### Controller.cpp

Controls the overall system logic and coordinates all components.

Important functions:

- `checkOrientation()`  
  Reads accelerometer data and determines device orientation.

- `updateClock()`  
  Retrieves current time from the RTC and updates the LCD.

- `updateCountdown()`  
  Handles countdown logic and updates timer values.

- `updateSettingDisplay()`  
  Updates the LCD when the user adjusts timer settings.

### Encoder.cpp

Handles communication with the KY-040 rotary encoder.

Important functions:

- `readRotation()`  
  Detects clockwise or counterclockwise rotation.

- `isPressed()`  
  Detects when the encoder push button is pressed.

### LCD.cpp

Controls the 16x2 LCD display.

Important functions:

- `moveCursor(unsigned char x, unsigned char y)`  
  Moves the LCD cursor to a selected position.

- `showClock(uint8_t hour, uint8_t minute, uint8_t second)`  
  Displays real-time clock information.

- `showCountdown(char mode, uint8_t minutes, uint8_t seconds)`  
  Displays countdown timer status.

### RTC.cpp

Handles communication with the DS1302 RTC module.

Important functions:

- `writeRegister(uint8_t address, uint8_t data)`  
  Writes data to an RTC register.

- `readRegister(uint8_t address)`  
  Reads data from an RTC register.

### Timer.cpp

Creates periodic timing signals for countdown logic.

Important function:

- `initTimer0()`  
  Configures Timer0 in CTC mode for periodic interrupts.

### ADXL345.cpp

Handles I2C communication with the ADXL345 accelerometer.

Important functions:

- `getOrientation()`  
  Interprets acceleration data and determines the current orientation.

- `readAccel()`  
  Reads raw X, Y, and Z acceleration data.

- `writeRegister(uint8_t reg, uint8_t value)`  
  Writes configuration values to ADXL345 registers.

### Buzzer.cpp

Controls the buzzer alarm.

Important functions:

- `init()`  
  Initializes the buzzer pin.

- `on()`  
  Turns the buzzer on.

- `off()`  
  Turns the buzzer off.

- `updatePattern()`  
  Controls the buzzer alarm pattern.

## Power Requirements

Estimated current draw during typical operation:

| Component | Estimated Current |
|---|---:|
| Arduino Mega 2560 | ~60 mA |
| 16x2 LCD with backlight | ~20 mA |
| KY-040 Rotary Encoder | ~0 mA |
| ADXL345 Accelerometer | ~0.2 mA |
| DS1302 RTC Module | ~1 mA |
| Passive Buzzer | 0–10 mA |

Estimated total current:

- Idle current: ~80–85 mA
- With buzzer active: ~90–105 mA

Power estimate:

```text
P = V × I
P = 9V × 0.09A
P ≈ 0.8W
