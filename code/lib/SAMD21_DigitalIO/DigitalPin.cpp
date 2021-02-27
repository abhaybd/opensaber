#include "DigitalPin.h"

DigitalPin::DigitalPin(int pin, int mode)
        : port(g_APinDescription[pin].ulPort), pinDescription(g_APinDescription[pin].ulPin), pin(pin), mode(mode) {
    pinMode(pin, mode);
}

bool DigitalPin::read() const {
    uint32_t pinMask = 1ul << pinDescription;
    return (PORT->Group[port].IN.reg & pinMask) != 0;
}

void DigitalPin::write(bool value) const {
    if (value) {
        on();
    } else {
        off();
    }
}

void DigitalPin::on() const {
    uint32_t pinMask = 1ul << pinDescription;
    PORT->Group[port].OUTSET.reg = pinMask;
}

void DigitalPin::off() const {
    uint32_t pinMask = 1ul << pinDescription;
    PORT->Group[port].OUTCLR.reg = pinMask;
}

int DigitalPin::getPin() const {
    return pin;
}

int DigitalPin::getMode() const {
    return mode;
}

void DigitalPin::setMode(int newMode) {
    mode = newMode;
    pinMode(pin, newMode);
}

void DigitalPin::write(int pin, bool value) {
    EPortType port = g_APinDescription[pin].ulPort;
    int pinDescription = g_APinDescription[pin].ulPin;
    uint32_t pinMask = 1ul << pinDescription;
    if (value) {
        PORT->Group[port].OUTSET.reg = pinMask;
    } else {
        PORT->Group[port].OUTCLR.reg = pinMask;
    }
}

bool DigitalPin::read(int pin) {
    EPortType port = g_APinDescription[pin].ulPort;
    int pinDescription = g_APinDescription[pin].ulPin;
    uint32_t pinMask = 1ul << pinDescription;
    return (PORT->Group[port].IN.reg & pinMask) != 0;
}
