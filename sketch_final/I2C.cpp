#include "I2C.h"
#include <avr/io.h>

#define F_CPU 16000000UL      // CPU clock frequency: 16 MHz
#define SCL_CLOCK 100000UL    // I2C clock frequency: 100 kHz
#define I2C_TIMEOUT 30000UL   // Timeout limit to prevent infinite waiting

// Wait until the I2C hardware finishes the current operation.
// Returns 1 if successful, 0 if timeout occurs.
static uint8_t waitForTWINT() {
    uint32_t count = 0;

    while (!(TWCR & (1 << TWINT))) {
        count++;

        if (count > I2C_TIMEOUT) {
            return 0;
        }
    }

    return 1;
}

// Initialize I2C hardware.
void I2C::init() {
    TWSR = 0x00;                              // Prescaler = 1
    TWBR = ((F_CPU / SCL_CLOCK) - 16) / 2;   // Set bit rate for 100 kHz I2C
    TWCR = (1 << TWEN);                      // Enable I2C module
}

// Send I2C START condition.
void I2C::start() {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    waitForTWINT();
}

// Send I2C STOP condition.
void I2C::stop() {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// Write one byte to the I2C bus.
void I2C::write(uint8_t data) {
    TWDR = data;                             // Load data into I2C data register
    TWCR = (1 << TWINT) | (1 << TWEN);       // Start transmission
    waitForTWINT();
}

// Read one byte and send ACK, meaning more bytes will be read.
uint8_t I2C::readAck() {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

    if (!waitForTWINT()) {
        return 0;
    }

    return TWDR;
}

// Read one byte and send NACK, meaning this is the last byte.
uint8_t I2C::readNack() {
    TWCR = (1 << TWINT) | (1 << TWEN);

    if (!waitForTWINT()) {
        return 0;
    }

    return TWDR;
}