#include "I2CBitBang.h"

// Reference:
// http://www.bitbanging.space/posts/bitbang-i2c
// https://forum.arduino.cc/index.php?topic=531268.0

I2CBitBang::I2CBitBang(uint8_t sdaPin, uint8_t sclPin)
        : sdaPin(sdaPin),
          sclPin(sclPin),
          sdaPort(g_APinDescription[sdaPin].ulPort),
          sclPort(g_APinDescription[sclPin].ulPort) {
    pinMode(sclPin, OUTPUT);
    pinMode(sdaPin, OUTPUT);
}

void I2CBitBang::start() {
    sdaOn();
    waitCycle();
    sclOn();
    waitCycle();
    sdaOff();
    waitCycle();
    sclOff();
    waitCycle();
}

void I2CBitBang::stop() {
    sdaOff();
    waitCycle();
    sclOn();
    waitCycle();
    sdaOn();
    waitCycle();
}

bool I2CBitBang::write(uint8_t data) {
    const uint8_t mask = 1 << 7;
    for (int i = 0; i < 8; i++) {
        // transmit the 8th bit
        if ((data & mask) != 0) {
            sdaOn();
        } else {
            sdaOff();
        }
        data <<= 1; // shift left by one
        waitCycle();
        pulseScl();
        waitCycle();
    }
    sdaOn();
    sclOn();
    waitCycle();
    bool ack = !sdaRead();
    sclOff();
    return ack;
}

uint8_t I2CBitBang::read(bool ack) {
    uint8_t data = 0;
    sdaOn();
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1; // shift left so we can write a new bit
        do {
            sclOn();
        } while (!sclRead()); // stretch that clock
        waitCycle();
        data |= sdaRead();
        waitCycle();
        sclOff();
    }
    if (ack) {
        sdaOff();
    } else {
        sdaOn();
    }
    pulseScl();
    sdaOn();
    return data;
}

bool I2CBitBang::sdaRead() {
    uint32_t pinMask = 1ul << g_APinDescription[sdaPin].ulPin;
    return (PORT->Group[sdaPort].IN.reg & pinMask) != 0;
}

void I2CBitBang::sdaOn() {
    uint32_t pinMask = 1ul << g_APinDescription[sdaPin].ulPin;
    PORT->Group[sdaPort].OUTSET.reg = pinMask;
}

void I2CBitBang::sdaOff() {
    uint32_t pinMask = 1ul << g_APinDescription[sdaPin].ulPin;
    PORT->Group[sdaPort].OUTCLR.reg = pinMask;
}

bool I2CBitBang::sclRead() {
    uint32_t pinMask = 1ul << g_APinDescription[sclPin].ulPin;
    return (PORT->Group[sclPort].IN.reg & pinMask) != 0;
}

void I2CBitBang::sclOn() {
    uint32_t pinMask = 1ul << g_APinDescription[sclPin].ulPin;
    PORT->Group[sclPort].OUTSET.reg = pinMask;
}

void I2CBitBang::sclOff() {
    uint32_t pinMask = 1ul << g_APinDescription[sclPin].ulPin;
    PORT->Group[sclPort].OUTCLR.reg = pinMask;
}

void I2CBitBang::pulseScl() {
    sclOn();
    waitCycle();
    sclOff();
}

void I2CBitBang::waitCycle() {
    __NOP();
}
