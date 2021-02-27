#include <Arduino.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>

#include "MPU6050_Gyro.h"

// import animation sounds
#include "IgnitionSound.h"

// dma requires pin 4
#define LED_PIN 4
#define NUM_LEDS 30
// currently using 30 leds, not doubled
#define LED_IDX_START 0
#define LED_IDX_MIDDLE 29
#define LED_IDX_END 29

#define AUDIO_PIN 1
#define DAC_PRECISION 10

#define arrLen(x) (sizeof(x) / sizeof((x)[0]))
#define US_PER_SEC 1000000ull

MPU6050_Gyro gyro(false, 0, 2);
Adafruit_NeoPixel_ZeroDMA leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
int color = 0x800000;

[[noreturn]] void end(bool flashError=true) {
    leds.clear();
    while (true) {
        if (flashError) {
            int errorColor = 0x800000;
            leds.fill(errorColor);
            leds.show();

            delay(1000);

            leds.clear();
            leds.show();
        }
    }
}

float getRotVel() {
    RotVel rotVel{};
    gyro.get(rotVel);
    // magnitude of axis-angle representation
    return hypotf(rotVel.x, rotVel.y); // TODO: might have to change these axes depending on the gyro orientation
}

uint trailingZeros(uint n) {
    uint c = 32;
    n &= -signed(n);
    if (n) c--;
    if (n & 0x0000FFFF) c -= 16;
    if (n & 0x00FF00FF) c -= 8;
    if (n & 0x0F0F0F0F) c -= 4;
    if (n & 0x33333333) c -= 2;
    if (n & 0x55555555) c -= 1;
    return c;
}

void writeAudio(uint value, int precision = 8) {
    analogWrite(AUDIO_PIN, value << (DAC_PRECISION - precision));
}

void ignite() {
    // ignite leds from bottom to top
    unsigned long long soundLen = arrLen(ignitionSound);
    uint soundTimeMicros = soundLen * US_PER_SEC / ignitionSoundFreq;
    int soundIdx = -1; // the last sound sample that has been played
    // these represent the smallest index that has not been lit up
    uint oldLed1Idx = 0; // counts up
    uint oldLed2Idx = LED_IDX_END; // counts down
    ulong time = micros();
    ulong start = time;

    while (time - start < soundTimeMicros) {
        uint elapsedTime = time - start;
        // the intermediate cast to ull is to prevent overflow from multiplication
        int newSoundIdx = static_cast<int>(static_cast<unsigned long long>(elapsedTime) * ignitionSoundFreq / US_PER_SEC);
        if (newSoundIdx > soundIdx) {
            soundIdx = newSoundIdx;
            writeAudio(ignitionSound[soundIdx]);

            // write to leds
            uint elapsedTimeLed = constrain(elapsedTime, 0, igniteTime);
            uint led1Idx = LED_IDX_START + ((elapsedTimeLed * (LED_IDX_MIDDLE - LED_IDX_START)) / igniteTime);
            uint led2Idx = LED_IDX_END - ((elapsedTimeLed * (LED_IDX_END - LED_IDX_MIDDLE)) / igniteTime);

            bool shouldShow = false;
            // led 1 counts up from START
            while (oldLed1Idx <= led1Idx) {
                leds.setPixelColor(oldLed1Idx++, color);
                shouldShow = true;
            }
            // led 2 counts down from END
            while (oldLed2Idx >= led2Idx) {
                leds.setPixelColor(oldLed2Idx--, color);
                shouldShow = true;
            }
            if (shouldShow) {
                leds.show();
            }
        }
        time = micros();
    }
}

void setup() {
//    while (!Serial); // for debugging
    if (!gyro.begin()) {
        Serial.println("Failed to find gyro!");
        end();
    }
    leds.begin();
    leds.clear();
    leds.show();
    delay(1000);
    Serial.println("Igniting now!");
    ignite(); // synchronously run the ignition routine
    leds.clear();
    leds.show();
}

void loop() {
    // TODO: implement the light and sound animations
}
