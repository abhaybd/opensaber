#ifndef CODE_I2CBITBANG_H
#define CODE_I2CBITBANG_H

#include <Arduino.h>
#include <DigitalPin.h>


class I2CBitBang {
public:
    I2CBitBang(uint8_t sdaPin, uint8_t sclPin);
    void start();
    void stop();
    bool write(uint8_t data);
    uint8_t read(bool ack);
private:
    void pulseScl();
    static void waitCycle();
    DigitalPin sda, scl;
};


#endif //CODE_I2CBITBANG_H
