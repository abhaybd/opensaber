#ifndef CODE_MPU6050_GYRO_H
#define CODE_MPU6050_GYRO_H

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

enum FilterBandwidth {
    HZ_256, HZ_188, HZ_98, HZ_42, HZ_20, HZ_10, HZ_5
};

class MPU6050_Gyro {
public:
    MPU6050_Gyro(uint8_t address, uint8_t sdaPin, uint8_t sclPin);

    ~MPU6050_Gyro();

    void begin();

    void end();

    void resetConfig();

    bool isInitialized();

    void get(RotVel &vel);

    void setBias(const RotVel &vel);

    void setScale(GyroScale gs);

    void setSampleRateDivisor(uint8_t divisor);

    void setGyroFilterBandwidth(FilterBandwidth bandwidth);

private:
    I2CBitBang *wire;
    uint8_t address;
    uint8_t sdaPin, sclPin;
    GyroScale gyroScale;
    RotVel bias;

    void registerWrite(uint8_t reg, uint8_t data);

    void registerWrite(uint8_t reg, uint8_t *data, uint len);

    void registerWriteBits(uint8_t reg, uint8_t data, uint8_t bitIdx, uint8_t len);

    uint8_t registerRead(uint8_t reg);

    void registerRead(uint8_t reg, uint8_t *data, uint len);
};


#endif //CODE_MPU6050_GYRO_H
