#include "Gyro.h"

#define WRITE_BIT 0
#define READ_BIT 1

Gyro::Gyro(uint8_t address, uint8_t sdaPin, uint8_t sclPin)
        : wire(nullptr), address(address << 1), sdaPin(sdaPin), sclPin(sclPin), // note that address is shifted
          scale(SCALE_250_DPS) {

}

Gyro::~Gyro() {
    if (isInitialized()) {
        delete wire;
    }
}

bool Gyro::isInitialized() {
    return wire != nullptr;
}

void Gyro::begin() {
    wire = new I2CBitBang(sdaPin, sclPin);
    setScale(scale);
    // TODO: maybe initialize sensor and write config? double check in register map or datasheet
}

void Gyro::end() {
    delete wire;
    wire = nullptr;
}

void Gyro::get(RotVel &vel) {
    // TODO: burst read from registers 67-72, then do unit conversions according to scale
}

void Gyro::setScale(GyroScale gs) {
    scale = gs;
    // TODO: refer to register map, possibly register 27?
}

void Gyro::registerWrite(uint8_t reg, uint8_t data) {
    // page 35 of datasheet
    wire->start();
    wire->write(address | WRITE_BIT);
    wire->write(reg);
    wire->write(data);
    wire->stop();
}

void Gyro::registerWrite(uint8_t reg, uint8_t *data, uint len) {
    // page 35 of datasheet
    wire->start();
    wire->write(address | WRITE_BIT);
    wire->write(reg);
    for (uint i = 0; i < len; i++) {
        wire->write(data[i]);
    }
    wire->stop();
}

uint8_t Gyro::registerRead(uint8_t reg) {
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

void Gyro::registerRead(uint8_t reg, uint8_t *data, uint len) {
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
