#include "MPU6050_Gyro.h"

#define WRITE_BIT 0
#define READ_BIT 1

#define SMPL_RATE_REG 25
#define CONFIG_REG 26
#define GYRO_CONFIG_REG 27
#define GYRO_DATA_REG 67
#define SIG_PATH_RST_REG 104
#define PWR_MGMT_1_REG 107
#define PWR_MGMT_2_REG 108
#define WHOAMI_REG 117

constexpr uint8_t defaultAddr = 0b1101000;
const float scaleFactors[] = {powf(2, 15) / 250.0f, powf(2, 15) / 500.0f, powf(2, 15) / 1000.0f, powf(2, 15) / 2000.0f};

MPU6050_Gyro::MPU6050_Gyro(bool addrLastBit, uint8_t sdaPin, uint8_t sclPin)
        : wire(nullptr), address((defaultAddr | addrLastBit) << 1), // note that the address is shifted left one bit
          sdaPin(sdaPin), sclPin(sclPin), gyroScale(SCALE_250_DPS), bias({0, 0, 0}) {

}

MPU6050_Gyro::~MPU6050_Gyro() {
    if (isInitialized()) {
        delete wire;
    }
}

bool MPU6050_Gyro::isInitialized() {
    return wire != nullptr;
}

bool MPU6050_Gyro::begin() {
    wire = new I2CBitBang(sdaPin, sclPin);

    uint8_t mask = ~1; // ignore last bit
    uint8_t whoami = whoAmI();
    if ((whoami & mask) != ((address >> 1) & mask)) {
        return false;
    }

    resetConfig();

    setSampleRateDivisor(0);
    setGyroFilterBandwidth(HZ_256);
    setScale(gyroScale);

    // configure clock to track gyro x
    registerWriteBits(PWR_MGMT_1_REG, 1, 0, 3);
    delay(100);

    // disable accelerometer and temp sensor
//    registerWriteBits(PWR_MGMT_1_REG, 1 << 3, 3, 1);
//    registerWriteBits(PWR_MGMT_2_REG, 0b111 << 3, 3, 3);
//    delay(100);
    return true;
}

void MPU6050_Gyro::end() {
    delete wire;
    wire = nullptr;
}

void MPU6050_Gyro::resetConfig() {
    registerWriteBits(PWR_MGMT_1_REG, 1 << 7, 7, 1);
    while ((registerRead(PWR_MGMT_1_REG) & (1 << 7)) != 0) {
        delay(1);
    }
    delay(100);
    registerWriteBits(SIG_PATH_RST_REG, 0b111, 0, 3);
    delay(100);
}

uint8_t MPU6050_Gyro::whoAmI() {
    uint8_t addr = registerRead(WHOAMI_REG);
    return addr;
}

void MPU6050_Gyro::get(RotVel &vel) {
    uint8_t data[6];
    registerRead(GYRO_DATA_REG, data, 6);
    for (uint8_t d : data) {
        Serial.print(d);
        Serial.print(", ");
    }
    Serial.println();
    int16_t x, y, z;
    memcpy(&x, &data[0], 2);
    memcpy(&y, &data[2], 2);
    memcpy(&z, &data[4], 2);
    float scaleFactor = scaleFactors[gyroScale];
    vel.x = static_cast<float>(x) / scaleFactor - bias.x;
    vel.y = static_cast<float>(y) / scaleFactor - bias.y;
    vel.z = static_cast<float>(z) / scaleFactor - bias.z;
}

void MPU6050_Gyro::setBias(const RotVel &vel) {
    bias = vel;
}

void MPU6050_Gyro::setScale(GyroScale gs) {
    gyroScale = gs;
    uint8_t data = (gs & 0b11) << 3; // bits 4 and 3 tell the data, everything else is zero
    registerWriteBits(GYRO_CONFIG_REG, data, 3, 2);

    uint8_t d = registerRead(GYRO_CONFIG_REG);
    Serial.print("Scale is correct: ");
    Serial.println(((d >> 3) & 0b11) == gs);
}

void MPU6050_Gyro::setSampleRateDivisor(uint8_t divisor) {
    registerWrite(SMPL_RATE_REG, divisor);
}

void MPU6050_Gyro::setGyroFilterBandwidth(FilterBandwidth bandwidth) {
    uint8_t data = bandwidth & 0b111;
    registerWriteBits(CONFIG_REG, data, 0, 3);
}

void MPU6050_Gyro::registerWriteBits(uint8_t reg, uint8_t data, uint8_t bitIdx, uint8_t len) {
    uint8_t currData = registerRead(reg);
    uint8_t mask = ((1 << len) - 1) << bitIdx;
    data = (currData & ~mask) | (data & mask);
    registerWrite(reg, data);
}

void MPU6050_Gyro::registerWrite(uint8_t reg, uint8_t data) {
    // page 35 of datasheet
    wire->start();
    wire->write(address | WRITE_BIT);
    wire->write(reg);
    wire->write(data);
    wire->stop();
}

void MPU6050_Gyro::registerWrite(uint8_t reg, uint8_t *data, uint len) {
    // page 35 of datasheet
    wire->start();
    wire->write(address | WRITE_BIT);
    wire->write(reg);
    for (uint i = 0; i < len; i++) {
        wire->write(data[i]);
    }
    wire->stop();
}

uint8_t MPU6050_Gyro::registerRead(uint8_t reg) {
    // page 36 of datasheet
    wire->start();
    wire->write(address | WRITE_BIT);
    wire->write(reg);
    wire->start();
    wire->write(address | READ_BIT);
    uint8_t data = wire->read(false); // NACK means end
    wire->stop();
    return data;
}

void MPU6050_Gyro::registerRead(uint8_t reg, uint8_t *data, uint len) {
    // page 36 of datasheet
    wire->start();
    wire->write(address | WRITE_BIT);
    wire->write(reg);
    wire->start();
    wire->write(address | READ_BIT);
    uint i = 0;
    while (i < len - 1) {
        data[i++] = wire->read(true); // ACK means keep sending
    }
    data[i] = wire->read(false); // NACK means end
    wire->stop();
}
