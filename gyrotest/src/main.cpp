#include <Arduino.h>
#include <MPU6050_Gyro.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_DotStar.h>

Adafruit_DotStar led(1, 7, 8, DOTSTAR_BGR);


[[noreturn]] void testSWI2C() {
    MPU6050_Gyro gyro(false, 0, 2);

    Serial.println("Testing SW I2C with gyro...");
    bool b = gyro.begin();
    if (!b) {
        Serial.println("Failed to boot up!");
        led.fill(0xFF0000);
        led.show();
        while (true);
    } else {
        led.fill(0x0000FF);
        led.show();
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
}

[[noreturn]] void testHWI2C() {
    Adafruit_MPU6050 mpu;
    Adafruit_MPU6050_Gyro gyro(&mpu);

    Serial.println("Testing HW I2C with gyro...");
    if (!mpu.begin()) {
        Serial.println("Failed to connect!");
        led.fill(0xFF0000);
        led.show();
        while (true);
    } else {
        sensors_event_t event;
        led.fill(0x0000FF);
        led.show();
        while (true) {
            delay(500);
            gyro.getEvent(&event);
            Serial.print("GYRO: X=");
            Serial.print(event.gyro.x);
            Serial.print(", Y=");
            Serial.print(event.gyro.y);
            Serial.print(", Z=");
            Serial.println(event.gyro.z);
        }
    }
}

void setup() {
// write your initialization code here
    Serial.begin(9600);
    led.begin();
    led.fill();
    led.show();
//    while (!Serial);

    delay(1000);

    testHWI2C();
//    testSWI2C();
}

void loop() {
//    Serial.println("Hello!");
//    delay(500);
}