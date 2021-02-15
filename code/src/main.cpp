#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_NeoPixel.h>

// import animation sounds
#include "IgnitionSound.h"

#define LED_PIN 8
#define NUM_LEDS 60
#define LED_IDX_START 0
#define LED_IDX_MIDDLE 30
#define LED_IDX_END 59

#define AUDIO_PIN 1
#define DAC_PRECISION 10

#define arrLen(x) (sizeof(x) / sizeof((x)[0]))

const uint igniteTime = 1000; // in millis

const int ignitionSoundFreq = 6000;

Adafruit_MPU6050 mpu;
Adafruit_MPU6050_Gyro gyro(&mpu);
Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
int color = 0xFF;

[[noreturn]] void end() {
    while (true);
}

float getRotVel() {
    sensors_event_t e;
    gyro.getEvent(&e);
    // magnitude of axis-angle representation
    return hypotf(e.gyro.x, e.gyro.y);
}

void writeAudio(uint value, int precision = 8) {
    analogWrite(AUDIO_PIN, value << (DAC_PRECISION - precision));
}

void ignite() {
    // ignite leds from bottom to top
    uint soundLen = arrLen(ignitionSound);
    float audioTime =
            static_cast<float>(soundLen) / static_cast<float>(ignitionSoundFreq); // NOLINT(bugprone-integer-division)
    ulong audioTimeMicros = lround(audioTime * 1000000);
    auto audioTimeMicrosF = static_cast<float>(audioTimeMicros);
    int oldSoundIdx = -1;
    int oldLed1Idx = -1;
    int oldLed2Idx = -1;
    ulong time;
    ulong start = micros();
    do {
        time = micros();
        auto elapsedTime = static_cast<float>(time - start); // in microseconds
        float audioProgress = elapsedTime / audioTimeMicrosF;
        int soundIdx = lround(audioProgress * soundLen);
        if (soundIdx > oldSoundIdx) {
            oldSoundIdx = soundIdx;
            writeAudio(ignitionSound[soundIdx]); // write sound to analog pin

            float ledProgressUnclamped = elapsedTime / static_cast<float>(igniteTime * 1000);
            float ledProgress = constrain(ledProgressUnclamped, 0.0f, 1.0f);
            int led1Idx = LED_IDX_START + lround(ledProgress * static_cast<float>(LED_IDX_MIDDLE - LED_IDX_START));
            if (led1Idx > oldLed1Idx) {
                oldLed1Idx = led1Idx;
                leds.setPixelColor(led1Idx, color);
            }
            int led2Idx = LED_IDX_END - lround(ledProgress * static_cast<float>(LED_IDX_END - LED_IDX_MIDDLE));
            if (led2Idx > oldLed2Idx) {
                oldLed2Idx = led2Idx;
                leds.setPixelColor(led2Idx, color);
            }
        }
    } while (time - start < audioTimeMicros);
}

void setup() {
    if (!mpu.begin()) {
        Serial.println("Failed to find gyro!");
        end();
    }
    leds.begin();
    ignite(); // synchronously run the ignition routine
}

void loop() {
    // TODO: implement the light and sound animations
}
