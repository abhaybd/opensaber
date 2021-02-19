#ifndef CODE_I2CBITBANG_H
#define CODE_I2CBITBANG_H

#include <Arduino.h>


class I2CBitBang {
public:
    I2CBitBang(uint8_t sdaPin, uint8_t sclPin);
private:
    void start();
    void stop();
    bool write(uint8_t data);
    uint8_t read(bool ack);
    bool sclRead();
    bool sdaRead();
    void sdaOn();
    void sclOn();
    void sdaOff();
    void sclOff();
    void pulseScl();
    static void waitCycle();
    uint8_t sdaPin, sclPin;
    EPortType sdaPort, sclPort;
};


#endif //CODE_I2CBITBANG_H
