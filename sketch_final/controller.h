#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include "RTC.h"
#include "Encoder.h"
#include "ADXL345.h"
#include "Buzzer.h"

enum PageState {
    PAGE_NORMAL,
    PAGE_SETTING,
    PAGE_EDIT_SEC,
    PAGE_EDIT_MIN,
    PAGE_CONFIRM,
    PAGE_DONE,

    PAGE_RTC_SETTING,
    PAGE_RTC_HOUR,
    PAGE_RTC_MIN,
    PAGE_RTC_CONFIRM
};

class Controller {
private:
    RTC rtc;
    Encoder encoder;
    ADXL345 accel;
    Buzzer buzzer;

    PageState pageState;
    Orientation lastOrientation;

    uint8_t setMinutes;
    uint8_t setSeconds;

    uint16_t remainingSeconds;
    uint8_t timerRunning;

    uint8_t settingComplete;
    uint8_t alarmActive;

    uint8_t lastButtonState;
    uint8_t blinkState;

    uint8_t rtcSetHour;
    uint8_t rtcSetMinute;

    uint8_t lastRTCButtonState;
    uint8_t rtcButtonLocked;

public:
    Controller();

    void init();
    void run();

private:
    void updateClock();
    void updateCountdown();

    void updateEncoder();
    void handleButtonPress();

    void updateSettingDisplay();

    void checkOrientation();

    void initRTCButton();
    uint8_t isRTCButtonPressed();
    void updateRTCButton();
    void updateRTCSettingDisplay();
};

#endif