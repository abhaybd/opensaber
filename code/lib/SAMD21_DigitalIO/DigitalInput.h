#ifndef CODE_DIGITALINPUT_H
#define CODE_DIGITALINPUT_H


#include "DigitalPin.h"

/**
 * A class encapsulating a digital input and some debouncing logic. Uses port manipulation for faster IO,
 * which means this only works on SAMD21 chips.
 */
class DigitalInput {
public:
    /**
     * Creates a new digital input and initializes the pin. (e.g. switch, button, etc)
     *
     * @param pin The pin connected to the digital input.
     * @param mode The input mode. One of INPUT, INPUT_PULLDOWN, INPUT_PULLUP.
     * @param debouncePeriodMillis The debounce period, in milliseconds. State changes will not be registered for these
     * many milliseconds following a state change.
     */
    DigitalInput(int pin, int mode, uint debouncePeriodMillis);

    /**
     * Gets the cached value of the state.
     *
     * @return True if the (debounced) current state is high, false if low. Undefined if floating.
     */
    bool getState() const;

    /**
     * Updates the state machine controlling the debounce. This method must be called frequently to poll the input.
     *
     * @return True if the current (debounced) state is high, false if low. Undefined if floating.
     */
    bool update();

    /**
     * Sets a new debounce period.
     *
     * @param newPeriod The new period for debouncing, in millis.
     */
    void setDebouncePeriod(uint newPeriod);

private:
    DigitalPin pin;
    ulong lastChangeMillis;
    uint debouncePeriod;
    bool currState;
};


#endif //CODE_DIGITALINPUT_H
