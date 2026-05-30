#include <avr/io.h>          // Provides AVR register definitions, such as DDRx, PORTx, PINx
#include <avr/interrupt.h>   // Provides interrupt functions/macros, especially sei()

#include "controller.h"      // Includes the Controller class declaration
#include "Timer.h"           // Includes initTimer0() and timer flag declarations

int main() {
    // Create the main controller object.
    // This object manages the LCD, RTC, encoder, ADXL345, buzzer, and system states.
    Controller controller;

    // Initialize all hardware modules controlled by the Controller.
    // This usually initializes LCD, RTC, encoder, accelerometer, buzzer, and buttons.
    controller.init();

    // Initialize Timer0.
    // Timer0 is configured to generate periodic interrupts, such as 1 ms timing ticks.
    initTimer0();

    // Enable global interrupts.
    // This must be called after timer initialization so Timer0 ISR can run.
    sei();

    // Main program loop.
    // The program runs forever and repeatedly checks inputs, updates display,
    // handles countdown logic, and responds to timer flags.
    while (1) {
        controller.run();
    }

    // This line is never reached in an embedded system with an infinite loop.
    return 0;
}