#include "DigitalInput.h"

#include <Arduino.h>

DigitalInput::DigitalInput(int pin, int mode, uint debouncePeriodMillis) : pin(pin, mode), debouncePeriod(debouncePeriodMillis) {
    currState = this->pin.read();
    lastChangeMillis = millis();
}

bool DigitalInput::getState() const {
    return currState;
}

bool DigitalInput::update() {
    bool state = pin.read();
    if (state != currState) {
        ulong time = millis();
        if (time - lastChangeMillis > debouncePeriod) {
            currState = state;
            lastChangeMillis = time;
        }
    }
    return currState;
}

void DigitalInput::setDebouncePeriod(uint newPeriod) {
    debouncePeriod = newPeriod;
}
