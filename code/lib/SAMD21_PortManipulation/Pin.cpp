#include "Pin.h"

Pin::Pin(int pin, int mode)
        : port(g_APinDescription[pin].ulPort), pinDescription(g_APinDescription[pin].ulPin), pin(pin), mode(mode) {
    pinMode(pin, mode);
}

bool Pin::read() const {
    uint32_t pinMask = 1ul << pinDescription;
    return (PORT->Group[port].IN.reg & pinMask) != 0;
}

void Pin::write(bool value) const {
    if (value) {
        on();
    } else {
        off();
    }
}

void Pin::on() const {
    uint32_t pinMask = 1ul << pinDescription;
    PORT->Group[port].OUTSET.reg = pinMask;
}

void Pin::off() const {
    uint32_t pinMask = 1ul << pinDescription;
    PORT->Group[port].OUTCLR.reg = pinMask;
}

int Pin::getPin() const {
    return pin;
}

int Pin::getMode() const {
    return mode;
}

void Pin::setMode(int newMode) {
    mode = newMode;
    pinMode(pin, newMode);
}

void Pin::write(int pin, bool value) {
    EPortType port = g_APinDescription[pin].ulPort;
    int pinDescription = g_APinDescription[pin].ulPin;
    uint32_t pinMask = 1ul << pinDescription;
    if (value) {
        PORT->Group[port].OUTSET.reg = pinMask;
    } else {
        PORT->Group[port].OUTCLR.reg = pinMask;
    }
}

bool Pin::read(int pin) {
    EPortType port = g_APinDescription[pin].ulPort;
    int pinDescription = g_APinDescription[pin].ulPin;
    uint32_t pinMask = 1ul << pinDescription;
    return (PORT->Group[port].IN.reg & pinMask) != 0;
}
