#ifndef CODE_GYRO_H
#define CODE_GYRO_H

#include <Arduino.h>
#include "I2CBitBang.h"

struct RotVel {
    float x;
    float y;
    float z;
};

enum GyroScale {
    SCALE_250_DPS, SCALE_500_DPS, SCALE_1000_DPS, SCALE_2000_DPS
};

class Gyro {
public:
    Gyro(uint8_t address, uint8_t sdaPin, uint8_t sclPin);
    ~Gyro();
    void get(RotVel &vel);
    void setScale(GyroScale gs);
    void begin();
    void end();
    bool isInitialized();
private:
    I2CBitBang *wire;
    uint8_t address;
    uint8_t sdaPin, sclPin;
    GyroScale scale;

    void registerWrite(uint8_t reg, uint8_t data);
    void registerWrite(uint8_t reg, uint8_t *data, uint len);
    uint8_t registerRead(uint8_t reg);
    void registerRead(uint8_t reg, uint8_t *data, uint len);
};


#endif //CODE_GYRO_H
