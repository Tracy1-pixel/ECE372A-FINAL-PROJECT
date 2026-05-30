#include "controller.h"
#include "LCD.h"
#include "Timer.h"
#include <avr/io.h>

// Mega2560 Digital Pin 3 = PE5
#define RTC_BUTTON_PIN   PE5
#define RTC_BUTTON_DDR   DDRE
#define RTC_BUTTON_PORT  PORTE
#define RTC_BUTTON_INPUT PINE

Controller::Controller() {
    pageState = PAGE_NORMAL;//initialize all variable and state
    lastOrientation = ORIENT_UNKNOWN;

    setMinutes = 0;
    setSeconds = 0;

    remainingSeconds = 0;
    timerRunning = 0;

    settingComplete = 0;
    alarmActive = 0;

    lastButtonState = 0;
    blinkState = 1;

    rtcSetHour = 0;
    rtcSetMinute = 0;

    lastRTCButtonState = 0;
    rtcButtonLocked = 0;
}

void Controller::init() {
    // Initialize hardware modules
    initLCD();
    rtc.init();
    encoder.init();
    accel.init();
    buzzer.init();
    initRTCButton();

    // Initial display
    updateClock();
    showCountdown("STOP", 0, 0);
}

void Controller::run() {
    // Check the dedicated RTC setting button
    updateRTCButton();

    // Encoder input must be checked continuously
    updateEncoder();

    // Run buzzer pattern every 100 ms while alarm is active
    if (g_100msFlag) {
        g_100msFlag = 0;//clear the flag justification

        if (alarmActive) {
            buzzer.updatePattern();//buzzer working
        }
    }

    if (g_1sFlag) {
        g_1sFlag = 0;//clear 1s flag justification

        if (pageState == PAGE_NORMAL || pageState == PAGE_DONE) {
            // RTC updates only on the normal / done pages
            updateClock();

            // ADXL345 only controls countdown start and alarm stop
            checkOrientation();

            if (timerRunning) {
                updateCountdown();
            }
        }
        else if (pageState == PAGE_RTC_SETTING ||
                 pageState == PAGE_RTC_HOUR ||
                 pageState == PAGE_RTC_MIN ||
                 pageState == PAGE_RTC_CONFIRM) {
            // RTC setting pages must use RTC display logic
            blinkState = !blinkState;
            updateRTCSettingDisplay();
        }
        else {
            // Alarm setting pages use alarm display logic
            blinkState = !blinkState;
            updateSettingDisplay();
        }
    }
}

void Controller::checkOrientation() {
    Orientation currentOrientation = accel.getOrientation();

    // Start countdown only when the device changes from lying to standing
    if (settingComplete &&
        lastOrientation == ORIENT_LYING &&
        currentOrientation == ORIENT_STANDING &&
        !timerRunning &&
        !alarmActive &&
        remainingSeconds > 0) {

        timerRunning = 1;
        settingComplete = 0;

        showCountdown("RUN", remainingSeconds / 60, remainingSeconds % 60);
    }

    // Stop alarm only when the device changes from standing to lying
    if (alarmActive &&
        lastOrientation == ORIENT_STANDING &&
        currentOrientation == ORIENT_LYING) {

        alarmActive = 0;
        settingComplete = 0;
        timerRunning = 0;
        remainingSeconds = 0;
        pageState = PAGE_NORMAL;

        buzzer.off();

        showCountdown("STOP", 0, 0);
    }

    lastOrientation = currentOrientation;
}

void Controller::updateEncoder() {
    // Disable encoder actions while alarm is active
    if (alarmActive) {
        return;
    }

    int8_t rotation = encoder.readRotation();

    // RTC hour setting
    if (pageState == PAGE_RTC_HOUR) {
        if (rotation == 1 && rtcSetHour < 23) {
            rtcSetHour++;
            updateRTCSettingDisplay();
        } else if (rotation == -1 && rtcSetHour > 0) {
            rtcSetHour--;
            updateRTCSettingDisplay();
        }

        uint8_t pressed = encoder.isPressed();

        if (pressed && !lastButtonState) {
            handleButtonPress();
        }

        lastButtonState = pressed;
        return;
    }

    // RTC minute setting
    if (pageState == PAGE_RTC_MIN) {
        if (rotation == 1 && rtcSetMinute < 59) {
            rtcSetMinute++;
            updateRTCSettingDisplay();
        } else if (rotation == -1 && rtcSetMinute > 0) {
            rtcSetMinute--;
            updateRTCSettingDisplay();
        }

        uint8_t pressed = encoder.isPressed();

        if (pressed && !lastButtonState) {
            handleButtonPress();
        }

        lastButtonState = pressed;
        return;
    }

    // Alarm seconds setting
    if (pageState == PAGE_EDIT_SEC) {
        if (rotation == 1 && setSeconds < 59) {
            setSeconds++;
            updateSettingDisplay();
        } else if (rotation == -1 && setSeconds > 0) {
            setSeconds--;
            updateSettingDisplay();
        }
    }

    // Alarm minutes setting
    if (pageState == PAGE_EDIT_MIN) {
        if (rotation == 1 && setMinutes < 99) {
            setMinutes++;
            updateSettingDisplay();
        } else if (rotation == -1 && setMinutes > 0) {
            setMinutes--;
            updateSettingDisplay();
        }
    }

    // Encoder push-button handling
    uint8_t pressed = encoder.isPressed();

    if (pressed && !lastButtonState) {
        handleButtonPress();
    }

    lastButtonState = pressed;
}

void Controller::handleButtonPress() {
    switch (pageState) {

    case PAGE_NORMAL:
        // Enter alarm setting page
        buzzer.off();
        timerRunning = 0;
        settingComplete = 0;
        alarmActive = 0;
        pageState = PAGE_SETTING;
        updateSettingDisplay();
        break;

    case PAGE_SETTING:
        // Enter alarm seconds editing
        pageState = PAGE_EDIT_SEC;
        updateSettingDisplay();
        break;

    case PAGE_EDIT_SEC:
        // Enter alarm minutes editing
        pageState = PAGE_EDIT_MIN;
        updateSettingDisplay();
        break;

    case PAGE_EDIT_MIN:
        // Enter alarm confirmation page
        pageState = PAGE_CONFIRM;
        updateSettingDisplay();
        break;

    case PAGE_CONFIRM:
        // Save selected alarm time but do not start countdown yet
        remainingSeconds = setMinutes * 60 + setSeconds;

        settingComplete = 1;
        timerRunning = 0;
        alarmActive = 0;
        pageState = PAGE_NORMAL;

        buzzer.off();

        updateClock();
        showCountdown("WAIT", remainingSeconds / 60, remainingSeconds % 60);
        break;

    case PAGE_RTC_SETTING:
        // Enter RTC hour editing
        pageState = PAGE_RTC_HOUR;
        updateRTCSettingDisplay();
        break;

    case PAGE_RTC_HOUR:
        // Enter RTC minute editing
        pageState = PAGE_RTC_MIN;
        updateRTCSettingDisplay();
        break;

    case PAGE_RTC_MIN:
        // Enter RTC confirmation page
        pageState = PAGE_RTC_CONFIRM;
        updateRTCSettingDisplay();
        break;

    case PAGE_RTC_CONFIRM:
        // Save RTC hour and minute
        rtc.setTime(rtcSetHour, rtcSetMinute, 0);

        pageState = PAGE_NORMAL;

        updateClock();
        showCountdown("STOP", 0, 0);
        break;

    default:
        break;
    }
}

void Controller::updateClock() {
    rtc.readTime();  // Read the latest time from the RTC module.

    uint8_t h = rtc.getHour();     // Get hour value.
    uint8_t m = rtc.getMinute();   // Get minute value.
    uint8_t s = rtc.getSecond();   // Get second value.

    // Check whether the RTC time values are valid.
    if (h > 23 || m > 59 || s > 59) {
        moveCursor(0, 0);          // Move to the first LCD row.
        writeString("RTC ERR ");   // Show RTC error message.
        return;                    // Stop updating the clock display.
    }

    showClock(h, m, s);            // Display time as HH:MM:SS.
}

void Controller::updateCountdown() {
    // Decrease the remaining time by 1 second if the countdown is not finished.
    if (remainingSeconds > 0) {
        remainingSeconds--;
    }

    // If the countdown reaches 0, stop the timer and activate the alarm.
    if (remainingSeconds == 0) {
        timerRunning = 0;       // Countdown is no longer running.
        alarmActive = 1;        // Enable alarm state.
        settingComplete = 0;    // Clear the setting-complete flag.
        pageState = PAGE_DONE;  // Switch to the done page.

        // Buzzer sound pattern is handled in run().
        showCountdown("DONE", 0, 0);
    } else {
        // Show the remaining time in minutes and seconds.
        showCountdown("RUN", remainingSeconds / 60, remainingSeconds % 60);
    }
}

void Controller::updateSettingDisplay() {
    moveCursor(0, 0);//move cursor on first line

    if (pageState == PAGE_CONFIRM) {//LCD display ALM setting page and confirm
        writeString("CONFIRM ");
    } else {
        writeString("SET ALM ");
    }

    moveCursor(0, 1);//move cursor on second line

    if (pageState == PAGE_EDIT_MIN && !blinkState) {
        writeString("  ");//blinking
    } else {
        writeCharacter((setMinutes / 10) + '0');// ten digit display
        writeCharacter((setMinutes % 10) + '0');//one digit
    }

    writeCharacter('m');//min
    writeCharacter(' ');

    if (pageState == PAGE_EDIT_SEC && !blinkState) {
        writeString("  ");
    } else {
        writeCharacter((setSeconds / 10) + '0');//ten digit display
        writeCharacter((setSeconds % 10) + '0');//one digit
    }

    writeCharacter('s');//second
}

void Controller::initRTCButton() {
    // Set RTC button pin as input
    RTC_BUTTON_DDR &= ~(1 << RTC_BUTTON_PIN);

    // Enable internal pull-up resistor
    RTC_BUTTON_PORT |= (1 << RTC_BUTTON_PIN);

    // Allow the pull-up voltage to stabilize briefly
    for (volatile uint16_t i = 0; i < 1000; i++);

    // Read the initial button state after enabling pull-up
    uint8_t pressed = isRTCButtonPressed();

    // If the button is already pressed during startup/reset,
    // lock it until it is released. This prevents auto-entering SET RTC.
    rtcButtonLocked = pressed;
    lastRTCButtonState = pressed;
}

uint8_t Controller::isRTCButtonPressed() {
    // Button is active LOW because the internal pull-up resistor is enabled
    return !(RTC_BUTTON_INPUT & (1 << RTC_BUTTON_PIN));
}

void Controller::updateRTCButton() {
    uint8_t pressed = isRTCButtonPressed();

    // If the button was pressed during startup or is still being held,
    // ignore it until it is released.
    if (rtcButtonLocked) {
        if (!pressed) {
            rtcButtonLocked = 0;
        }

        lastRTCButtonState = pressed;
        return;
    }

    // Handle one clean button press event
    if (pressed && !lastRTCButtonState) {
        rtcButtonLocked = 1;

        // RTC setting can only be entered from idle normal page
        if (pageState == PAGE_NORMAL &&
            !timerRunning &&
            !alarmActive &&
            !settingComplete) {

            rtc.readTime();

            rtcSetHour = rtc.getHour();
            rtcSetMinute = rtc.getMinute();

            pageState = PAGE_RTC_SETTING;
            updateRTCSettingDisplay();
        }
    }

    lastRTCButtonState = pressed;
}

void Controller::updateRTCSettingDisplay() {
    moveCursor(0, 0);  // Move to the first LCD row.

    if (pageState == PAGE_RTC_CONFIRM) {
        writeString("RTC OK  ");  // Show RTC confirmation text.
    } else {
        writeString("SET RTC ");  // Show RTC setting text.
    }

    moveCursor(0, 1);  // Move to the second LCD row.

    // Blink the hour field when editing RTC hour.
    if (pageState == PAGE_RTC_HOUR && !blinkState) {
        writeString("  ");//show blank so that when u setting hour the digit will blinking
    } else {
        writeCharacter((rtcSetHour / 10) + '0');  // Hour tens digit.
        writeCharacter((rtcSetHour % 10) + '0');  // Hour ones digit.
    }

    writeCharacter(':');  // Time separator.

    // Blink the minute field when editing RTC minute.
    if (pageState == PAGE_RTC_MIN && !blinkState) {
        writeString("  ");//show blank so that when u setting hour the digit will blinking
    } else {
        writeCharacter((rtcSetMinute / 10) + '0');  // Minute tens digit display.
        writeCharacter((rtcSetMinute % 10) + '0');  // Minute ones digit display.
    }

    writeString("   ");  // Clear leftover characters.
}