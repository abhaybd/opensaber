#ifndef CODE_PIN_H
#define CODE_PIN_H

#include <Arduino.h>


class Pin {
public:
    /**
     * Create a new Pin object and initialize the physical pin to the given mode.
     * @param pin The pin referred to by this object.
     * @param mode The mode to use. One of (OUTPUT, INPUT, INPUT_PULLUP, INPUT_PULLDOWN)
     */
    Pin(int pin, int mode);

    /**
     * Reads the current pin state. This works even if this is an output pin.
     *
     * @return True if the pin is pulled high, false if low. Undefined if floating.
     */
    bool read() const;

    /**
     * Write the given value to this pin. Probably only works for output pins.
     *
     * @param value If true, set the pin high. If false, set low.
     */
    void write(bool value) const;

    /**
     * Sets this pin high. Probably only works for output pins.
     */
    void on() const;

    /**
     * Sets this pin low. Probably only works for output pins.
     */
    void off() const;

    /**
     * Gets the pin referred to by this pin object.
     * @return The pin number.
     */
    int getPin() const;

    /**
     * Gets the current mode of this pin.
     * @return The mode of this pin.
     */
    int getMode() const;

    /**
     * Sets this pin to a new mode.
     * @param newMode The new mode. One of OUTPUT, INPUT, INPUT_PULLDOWN, INPUT_PULLUP.
     */
    void setMode(int newMode);

    /**
     * Write the given value to the given pin. Probably only works for output pins.
     * The pin is assumed to have been be initialized correctly.
     *
     * @param pin The pin to write to.
     * @param value If true, set the pin high. If false, set low.
     */
    static void write(int pin, bool value);

    /**
     * Reads the current state of the given pin. This works even if this is an output pin.
     * The pin is assumed to have been be initialized correctly.
     *
     * @param pin The pin to read.
     * @return True if the pin is pulled high, false if low. Undefined if floating.
     */
    static bool read(int pin);
private:
    EPortType port;
    int pinDescription;
    int pin;
    int mode;
};


#endif //CODE_PIN_H
