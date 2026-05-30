#include "ADXL345.h"
#include "I2C.h"

#define ADXL345_ADDR 0x53

#define REG_POWER_CTL   0x2D
#define REG_DATA_FORMAT 0x31
#define REG_DATAX0      0x32

ADXL345::ADXL345() {//Initialization
    x = 0;
    y = 0;
    z = 0;
}

void ADXL345::writeRegister(uint8_t reg, uint8_t value) {
    I2C::start();                   // Start I2C communication.

    I2C::write(ADXL345_ADDR << 1);  // Send ADXL345 address in write mode.
                                      // Left shift leaves the last bit as 0 for write.

    I2C::write(reg);                // Send the register address to write to.

    I2C::write(value);              // Send the value that will be written into the register.

    I2C::stop();                    // Stop I2C communication.
}

void ADXL345::init() {
    I2C::init();

    // Full resolution mode, +/-2g range
    writeRegister(REG_DATA_FORMAT, 0x08);

    // Measurement mode
    writeRegister(REG_POWER_CTL, 0x08);
}

void ADXL345::readAccel() {
    uint8_t data[6];  // Store 6 bytes: X0, X1, Y0, Y1, Z0, Z1.

    I2C::start();                         // Start I2C communication.
    I2C::write(ADXL345_ADDR << 1);        // Send ADXL345 address in write mode.
    I2C::write(REG_DATAX0);               // Set read start register to DATAX0.

    I2C::start();                         // Repeated start for reading.
    I2C::write((ADXL345_ADDR << 1) | 1);  // Send ADXL345 address in read mode.

    data[0] = I2C::readAck();   // Read X low byte, then ACK.
    data[1] = I2C::readAck();   // Read X high byte, then ACK.
    data[2] = I2C::readAck();   // Read Y low byte, then ACK.
    data[3] = I2C::readAck();   // Read Y high byte, then ACK.
    data[4] = I2C::readAck();   // Read Z low byte, then ACK.
    data[5] = I2C::readNack();  // Read Z high byte, then NACK because it is the last byte.

    I2C::stop();  // End I2C communication.

    // Combine low and high bytes into signed 16-bit acceleration values.
    x = (int16_t)((data[1] << 8) | data[0]);
    y = (int16_t)((data[3] << 8) | data[2]);
    z = (int16_t)((data[5] << 8) | data[4]);
}

Orientation ADXL345::getOrientation() {
    readAccel();

    int16_t absX = (x >= 0) ? x : -x;
    int16_t absY = (y >= 0) ? y : -y;
    int16_t absZ = (z >= 0) ? z : -z;

    // Lying flat: Z-axis has the strongest gravity component
    if (absZ > absX && absZ > absY && absZ > 150) {
        return ORIENT_LYING;
    }

    // Standing upright: only one X-axis direction is accepted
    // If the standing direction is opposite, change x < -150 to x > 150.
    if (x < -150 && absX > absY && absX > absZ) {
        return ORIENT_STANDING;
    }

    return ORIENT_UNKNOWN;
}

int16_t ADXL345::getX() {
    return x;  // Return the latest X-axis acceleration value.
}

int16_t ADXL345::getY() {
    return y;  // Return the latest Y-axis acceleration value.
}

int16_t ADXL345::getZ() {
    return z;  // Return the latest Z-axis acceleration value.
}