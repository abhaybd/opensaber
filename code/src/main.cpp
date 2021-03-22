#include <Arduino.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>

#include <cmath>

#include "MPU6050_Gyro.h"

#undef roundf

// import animation sounds
#include "IgnitionSound.h"
#include "HumSound.h"

//#define DEBUG

// dma requires pin 4
constexpr int LED_PIN = 4;
// currently using 30 leds, not doubled
constexpr int NUM_LEDS = 30;
constexpr int LED_IDX_START = 0;
constexpr int LED_IDX_MIDDLE = 29;

constexpr int LED_IDX_END = 29;
constexpr int AUDIO_PIN = A0;

constexpr int DAC_PRECISION = 10;

constexpr bool GYRO_OPTIONAL = false;

constexpr uint64_t US_PER_SEC = 1000000ull;

MPU6050_Gyro gyro(false, 0, 2);
Adafruit_NeoPixel_ZeroDMA leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int color = 0xFF0000;
bool gyroInitialized = false;

constexpr uint8_t baselineBrightness = 128; // unitless, out of 255
constexpr uint8_t maxBrightness = 255; // unitless, out of 255
constexpr ulong gyroUpdatePeriod = 50000; // microseconds
constexpr float maxRotVel = 400; // degrees per second

template<typename T>
T clamp(T val, T low, T high) {
    return val < low ? low : (val > high ? high : val);
}

template<typename T, int size>
constexpr uint arrLen(T(&)[size]) {
    return size;
}

[[noreturn]] void end(bool flashError = true) {
    leds.clear();
    while (true) {
        if (flashError) {
            int errorColor = 0x800000;
            leds.fill(errorColor);
            leds.show();
            delay(1000);

            leds.clear();
            leds.show();
            delay(1000);
        }
    }
}

float getRotVel() {
    RotVel rotVel{};
    gyro.get(rotVel);
    // magnitude of axis-angle representation
    return hypotf(rotVel.x, rotVel.y); // TODO: might have to change these axes depending on the gyro orientation
}

void writeAudio(uint value, int precision = 8) {
    analogWrite(AUDIO_PIN, value << (DAC_PRECISION - precision));
}

void ignite() {
    // ignite leds from bottom to top
    constexpr uint64_t soundLen = arrLen(ignitionSound);
    uint soundTimeMicros = soundLen * US_PER_SEC / ignitionSoundFreq;
    int soundIdx = -1; // the last sound sample that has been played
    // these represent the smallest index that has not been lit up
    uint oldLed1Idx = 0; // counts up
    uint oldLed2Idx = LED_IDX_END; // counts down
    ulong time = micros();
    ulong start = time;

    int numPlayed = 0;

    while (time - start < soundTimeMicros) {
        uint elapsedTime = time - start;
        // the intermediate cast to ull is to prevent overflow from multiplication
        int newSoundIdx = static_cast<int>(static_cast<uint64_t>(elapsedTime) * ignitionSoundFreq / US_PER_SEC);
        if (newSoundIdx > soundIdx) {
            soundIdx = newSoundIdx;
            numPlayed++;
            writeAudio(ignitionSound[soundIdx]);

            // write to leds
            uint elapsedTimeLed = clamp(elapsedTime, 0u, igniteTime);
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

    // verify that all audio samples were played
    Serial.print("Played ");
    Serial.print(numPlayed);
    Serial.print(" audio samples out of ");
    Serial.println(static_cast<int>(soundLen));

    leds.fill(color, LED_IDX_START, LED_IDX_END - LED_IDX_START + 1);
    leds.show();
}

uint8_t transformAudio(float rotVel, uint8_t sample) {
    auto unbiased = static_cast<float>(sample - 128);
    float scaled = roundf(unbiased * (1.0f + rotVel / maxRotVel));
    scaled = clamp(scaled, -128.0f, 127.0f);
    return static_cast<uint8_t>(scaled + 128.0f);
}

void lightsaberLoop() {
    static ulong start = micros();
    static float rotVel = 0;
    ulong time = micros();

    static ulong lastGyroUpdate = micros();
    if (gyroInitialized && time - lastGyroUpdate >= gyroUpdatePeriod) {
        lastGyroUpdate = time;
        rotVel = clamp(getRotVel(), 0.0f, maxRotVel);

        float additionalBrightness = (rotVel / maxRotVel) * (maxBrightness - baselineBrightness);
        leds.setBrightness(baselineBrightness + static_cast<int>(roundf(additionalBrightness)));
        leds.show();
    }

    static int oldSoundIdx = -1;
    constexpr uint64_t audioLenMicros = arrLen(humSound) * US_PER_SEC / humSoundFreq;
    ulong audioTime = (time - start) % audioLenMicros;
    int soundIdx = static_cast<int>(static_cast<uint64_t>(audioTime) * humSoundFreq / US_PER_SEC);
    if (soundIdx != oldSoundIdx) {
        writeAudio(transformAudio(rotVel, humSound[soundIdx]));
        oldSoundIdx = soundIdx;
    }
}

void setup() {
#ifdef DEBUG
    while (!Serial); // for debugging
#endif
    leds.begin();
    leds.setBrightness(baselineBrightness);
    leds.clear();
    leds.show();
    gyroInitialized = true;
    if (!gyro.begin()) {
        gyroInitialized = false;
        Serial.println("Failed to find gyro!");
        if (!GYRO_OPTIONAL) {
            end();
        }
    }
    delay(1000);
    Serial.println("Igniting now!");
    ignite(); // synchronously run the ignition routine

//#ifdef DEBUG
    // just for development, to save power
    leds.clear();
    leds.show();
    end(false);
//#endif
}

void loop() {
    lightsaberLoop();
}
