#include <Arduino.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>

#include <cmath>

#include "MPU6050_Gyro.h"

#undef roundf

// import animation sounds
#include "IgnitionSound.h"
#include "HumSound.h"
#include "Extinguish.h"

//#define DEBUG
//#define GYRO_OPTIONAL

// dma requires pin 4
constexpr int LED_PIN = 4;

// 60 LEDS, where the middle of the strip is the tip of the blade, and the ends of the strip is the base
constexpr int NUM_LEDS = 59;
constexpr int LED_IDX_START = 0;
constexpr int LED_IDX_MIDDLE = 29;
constexpr int LED_IDX_END = 58;

constexpr int AUDIO_PIN = A0;

constexpr int BUTTON_PIN = 3;

constexpr int DAC_PRECISION = 10;

constexpr uint64_t US_PER_SEC = 1000000ull;

MPU6050_Gyro gyro(false, 0, 2);
Adafruit_NeoPixel_ZeroDMA leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

constexpr uint8_t baselineBrightness = 128; // unitless, out of 255
constexpr uint8_t maxBrightness = 255; // unitless, out of 255
constexpr ulong gyroUpdatePeriod = 50000; // microseconds
constexpr float maxRotVel = 200; // degrees per second
float humMaxScaleFactor; // initialized in setup()
constexpr uint CHANGE_COLOR_PRESS_DURATION = 1000; // ms
constexpr uint EXTINGUISH_PRESS_DURATION = 50; // ms
const int colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};

// state variables
uint8_t colorIndex = 0;
int color = colors[colorIndex];
bool gyroInitialized = false;
bool shouldExtinguish = false;
ulong buttonPressedTime = millis();

template<typename T>
float maxScaleFactor(uint size, const T sound[], float maxVal) {
    if (size == 0) return 1;
    T max = sound[0];
    for (uint i = 1; i < size; i++) {
        if (max < sound[i]) max = sound[i];
    }
    return max == 0 ? 1 : maxVal / static_cast<float>(max);
}

template<typename T>
T clamp(T val, T low, T high) {
    return val < low ? low : (val > high ? high : val);
}

template<typename T, int size>
constexpr uint arrLen(T(&)[size]) {
    return size;
}

void writeAudio(uint value, int precision) {
    // shift either left or right as necessary
    if (precision > DAC_PRECISION) {
        value >>= precision - DAC_PRECISION;
    } else if (precision < DAC_PRECISION) {
        value <<= DAC_PRECISION - precision;
    }
    // this method internally masks the 10 LSB, so we don't have to
    analogWrite(AUDIO_PIN, value);
}

[[noreturn]] void end(bool flashError = true) {
    leds.clear();
    writeAudio(0, DAC_PRECISION);
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
    return hypotf(rotVel.x, rotVel.z); // we don't care about y rotation, since that's just roll
}

// extinguish and turn off the blade
void extinguish() {
    // disable interrupt while extinguishing
    detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));

    // extinguish leds from top to bottom
    constexpr uint64_t soundLen = arrLen(extinguishSound.sound);
    uint soundTimeMicros = soundLen * US_PER_SEC / extinguishSound.freq;
    int soundIdx = -1; // the last sound sample that has been played
    // these represent the largest and smallest indexes that have not yet been turned off
    int oldLed1Idx = LED_IDX_MIDDLE; // counts down
    int oldLed2Idx = LED_IDX_MIDDLE; // counts down
    ulong time = micros();
    ulong start = time;
    constexpr int OFF_COLOR = 0;
    constexpr uint extinguishTime = extinguishEnd - extinguishStart;

    while (time - start < soundTimeMicros) {
        uint elapsedTime = time - start;
        // the intermediate cast to ull is to prevent overflow from multiplication
        int newSoundIdx = static_cast<int>(static_cast<uint64_t>(elapsedTime) * extinguishSound.freq / US_PER_SEC);
        if (newSoundIdx > soundIdx) {
            soundIdx = newSoundIdx;
            writeAudio(extinguishSound.sound[soundIdx], extinguishSound.precision);

            if (elapsedTime >= extinguishStart) {
                // write to leds
                uint elapsedTimeLed = clamp(elapsedTime - extinguishStart, 0u, extinguishTime);
                // interpolate the elapsed time between the middle and both the start and end
                int led1Idx = static_cast<int>(
                        LED_IDX_MIDDLE - ((elapsedTimeLed * (LED_IDX_MIDDLE - LED_IDX_START)) / extinguishTime));
                int led2Idx = static_cast<int>(
                        LED_IDX_MIDDLE + ((elapsedTimeLed * (LED_IDX_END - LED_IDX_MIDDLE)) / extinguishTime));

                bool shouldShow = false;
                // led 1 counts down from MIDDLE
                while (oldLed1Idx >= led1Idx) {
                    leds.setPixelColor(oldLed1Idx--, OFF_COLOR);
                    shouldShow = true;
                }
                // led 2 counts up from MIDDLE
                while (oldLed2Idx <= led2Idx) {
                    leds.setPixelColor(oldLed2Idx++, OFF_COLOR);
                    shouldShow = true;
                }
                if (shouldShow) {
                    leds.show();
                }
            }
        }
        time = micros();
    }

    leds.clear();
    leds.show();
}

void buttonInterrupt() {
    // pressed = low, released = high
    if (digitalRead(BUTTON_PIN) == LOW) {
        buttonPressedTime = millis();
    } else {
        ulong elapsed = millis() - buttonPressedTime;
        if (elapsed >= CHANGE_COLOR_PRESS_DURATION) {
            colorIndex = (colorIndex + 1) % arrLen(colors);
        } else if (elapsed >= EXTINGUISH_PRESS_DURATION) {
            shouldExtinguish = true;
        }
    }
}

void ignite() {
    // ignite leds from bottom to top
    constexpr uint64_t soundLen = arrLen(ignitionSound.sound);
    uint soundTimeMicros = soundLen * US_PER_SEC / ignitionSound.freq;
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
        int newSoundIdx = static_cast<int>(static_cast<uint64_t>(elapsedTime) * ignitionSound.freq / US_PER_SEC);
        if (newSoundIdx > soundIdx) {
            soundIdx = newSoundIdx;
            numPlayed++;
            writeAudio(ignitionSound.sound[soundIdx], ignitionSound.precision);

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

// Scales a given (biased) audio data point relative to the rotVel.
uint transformHumAudio(float rotVel, uint sample, uint precision) {
    // convert to unbiased, perform scaling, convert back to biased
    auto biasF = static_cast<float>(1 << (precision - 1));
    auto unbiased = static_cast<float>(sample) - biasF;
    float scaleFactor = 1.0f + (humMaxScaleFactor - 1.0f) * rotVel / maxRotVel;
    float scaled = roundf(unbiased * scaleFactor);
    scaled = clamp(scaled, -biasF, biasF - 1);
    return static_cast<uint>(scaled + biasF);
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
        uint8_t brightness = baselineBrightness + static_cast<uint8_t>(roundf(additionalBrightness));
        leds.setBrightness(Adafruit_NeoPixel_ZeroDMA::gamma8(brightness));
        leds.show();
    }

    static int oldSoundIdx = -1;
    constexpr uint64_t audioLenMicros = arrLen(humSound.sound) * US_PER_SEC / humSound.freq;
    ulong audioTime = (time - start) % audioLenMicros;
    int soundIdx = static_cast<int>(static_cast<uint64_t>(audioTime) * humSound.freq / US_PER_SEC);
    if (soundIdx != oldSoundIdx) {
        uint val = transformHumAudio(rotVel, humSound.sound[soundIdx], humSound.precision);
        writeAudio(val, humSound.precision);
        oldSoundIdx = soundIdx;
    }
}

void setup() {
#ifdef DEBUG
    while (!Serial); // for debugging
#endif
    leds.begin();
    leds.setBrightness(Adafruit_NeoPixel_ZeroDMA::gamma8(baselineBrightness));
    leds.clear();
    leds.show();
    gyroInitialized = true;
    if (!gyro.begin()) {
        gyroInitialized = false;
        Serial.println("Failed to find gyro!");
#ifndef GYRO_OPTIONAL
        end();
#endif
    }
    humMaxScaleFactor = maxScaleFactor(arrLen(humSound.sound), humSound.sound,
                                       static_cast<float>((1 << humSound.precision) - 1));
    delay(1000);
    Serial.println("Igniting now!");
    ignite(); // synchronously run the ignition routine

//    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, CHANGE);

//#ifdef DEBUG
    // just for development, to save power
//#endif
//    leds.clear();
//    leds.show();
//    end(false);
}

void loop() {
    // TODO: get rid of the code that turns off the blade after some time
    static ulong start = millis();
    // if necessary, extinguish and then end
    if (shouldExtinguish || (millis() - start) >= 5000) {
        extinguish();
        end(false);
    }
    if (colors[colorIndex] != color) {
        color = colors[colorIndex];
        leds.fill(color);
        leds.show();
    }
    lightsaberLoop();
}
