#include <Arduino.h>
#include <MPU6050_Gyro.h>

MPU6050_Gyro gyro(false, 0, 2);

[[noreturn]] void foo() {
    Serial.println("Starting up!");
    bool b = gyro.begin();
    RotVel vel{};
    while (true) {
        ulong start = micros();
        gyro.get(vel);
        ulong elapsed = micros() - start;
        Serial.print("X: ");
        Serial.print(vel.x);
        Serial.print(", Y: ");
        Serial.print(vel.y);
        Serial.print(", Z: ");
        Serial.print(vel.z);
        Serial.println();
        Serial.print("Elapsed: ");
        Serial.print(elapsed);
        Serial.println("us");
        delay(100);
    }
}

void setup() {
// write your initialization code here
Serial.begin(9600);
while (!Serial);
foo();
//    int pin = 3;
//pinMode(pin, OUTPUT);
//    EPortType port = g_APinDescription[pin].ulPort;
//    uint32_t pinMask = 1ul << g_APinDescription[pin].ulPin;
//    PORT->Group[port].OUTSET.reg = pinMask;
//digitalWrite(pin, HIGH);
}

void loop() {
    Serial.println("Hello!");
    delay(500);
}