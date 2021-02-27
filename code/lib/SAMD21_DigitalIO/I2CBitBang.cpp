#include "I2CBitBang.h"

// Reference:
// http://www.bitbanging.space/posts/bitbang-i2c
// https://forum.arduino.cc/index.php?topic=531268.0

I2CBitBang::I2CBitBang(uint8_t sdaPin, uint8_t sclPin)
        : sda(sdaPin, OUTPUT), scl(sclPin, OUTPUT) {
}

void I2CBitBang::start() {
    sda.on();
    waitCycle();
    scl.on();
    waitCycle();
    sda.off();
    waitCycle();
    scl.off();
    waitCycle();
}

void I2CBitBang::stop() {
    sda.off();
    waitCycle();
    scl.on();
    waitCycle();
    sda.on();
    waitCycle();
}

bool I2CBitBang::write(uint8_t data) {
    const uint8_t mask = 1 << 7;
    for (int i = 0; i < 8; i++) {
        // transmit the 8th bit
        if ((data & mask) != 0) {
            sda.on();
        } else {
            sda.off();
        }
        data <<= 1; // shift left by one
        waitCycle();
        pulseScl();
        waitCycle();
    }
    sda.on();
    scl.on();
    waitCycle();
    bool ack = !sda.read();
    scl.off();
    return ack;
}

uint8_t I2CBitBang::read(bool ack) {
    uint8_t data = 0;
    sda.on();
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1; // shift left so we can write a new bit
        do {
            scl.on();
        } while (!scl.read()); // stretch that clock
        waitCycle();
        data |= sda.read();
        waitCycle();
        scl.off();
    }
    if (ack) {
        sda.off();
    } else {
        sda.on();
    }
    pulseScl();
    sda.on();
    return data;
}

void I2CBitBang::pulseScl() {
    scl.on();
    waitCycle();
    scl.off();
}

void I2CBitBang::waitCycle() {
    __NOP();
}
