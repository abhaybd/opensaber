#ifndef CODE_MPU6050_GYRO_H
#define CODE_MPU6050_GYRO_H

#include <Arduino.h>
#include <I2CBitBang.h>

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

/**
 * Represents a MPU6050 gyro-only sensor. Technically, this sensor also has an accelerometer and temp sensor,
 * but those are disabled, and this only exposes gyro functionality.
 */
class MPU6050_Gyro {
public:
    /**
     * Create a new MPU6050 gyro-only sensor.
     *
     * @param addrLastBit The last bit of the I2C address, as determined by the logic level of the appropriate pin.
     * @param sdaPin The GPIO pin to be used as SDA.
     * @param sclPin The GPIO pin to be used as SCL.
     */
    MPU6050_Gyro(bool addrLastBit, uint8_t sdaPin, uint8_t sclPin);

    ~MPU6050_Gyro();

    /**
     * Initializes the gyro sensor. This takes a while. Do NOT call multiple times without calling end() in the middle.
     */
    bool begin();

    /**
     * CLeans up and releases the resources associated with this gyro sensor. Calling any member functions that use
     * the sensor after calling this method is UB.
     */
    void end();

    /**
     * Resets the configurations of the gyro. This takes a while, so don't call in a loop.
     */
    void resetConfig();

    /**
     * Checks if the sensor has been initialized.
     *
     * @return True if the sensor is ready and calling the sensor functions is allowed.
     */
    bool isInitialized();

    /**
     * Gets the rotational velocity of the gyro.
     *
     * @param vel The rotational velocity, in degrees per second.
     */
    void get(RotVel &vel);

    /**
     * Configure the bias of the sensor. This value is subtracted from the sensor readings before being returned
     * by get().
     *
     * @param vel The bias of the gyro sensor.
     */
    void setBias(const RotVel &vel);

    /**
     * Configure the scale of the sensor. Changes the range of values returned in get(). Higher ranges means less
     * precision.
     *
     * @param gs The new gyro scale.
     */
    void setScale(GyroScale gs);

    /**
     * Configures the sample rate of the gyro sensor by changing the divisor.
     * sampleRate = gyroRate / (1 + divisor)
     *
     * @param divisor The new divisor.
     */
    void setSampleRateDivisor(uint8_t divisor);

    /**
     * Configures the low-pass filter bandwidth. The bandwidths are slightly different for the accel and gyro, but
     * this is a gyro-only, so it's all good.
     *
     * @param bandwidth The new bandwidth to set.
     */
    void setGyroFilterBandwidth(FilterBandwidth bandwidth);

    /**
     * Returns the I2C base address for sanity checks.
     *
     * @return The base I2C address, or 0b1101000 as defined by the spec sheet. In reality, the address' last bit
     * may be a 0 or a 1, but the returned will always be a 0.
     */
    uint8_t whoAmI();

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
