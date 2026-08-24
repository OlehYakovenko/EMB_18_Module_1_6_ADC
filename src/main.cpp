#include <Arduino.h>

#define LDR_PIN 4
#define RED_LED_PIN 15
#define GREEN_LED_PIN 16
#define BUTTON_PIN 17

const unsigned long DEBOUNCE_DELAY = 50;

bool swapped = false;
uint8_t buttonLastReading = HIGH, buttonState = HIGH;
unsigned long buttonDebounceTime = 0;

int rawMin, rawMax; 
bool isBright = true;

bool pressed(uint8_t pin, uint8_t &lastReading, uint8_t &state, unsigned long &debounceTime) {
    uint8_t reading = digitalRead(pin);
    if (reading != lastReading) {
        debounceTime = millis();
    }
    lastReading = reading;

    if (millis() - debounceTime > DEBOUNCE_DELAY && reading != state) {
        state = reading;
        return state == LOW;
    }
    return false;
}

int readRawAveraged() {
    const int SAMPLES = 8;
    long sum = 0;
    for (int i = 0; i < SAMPLES; i++) {
        sum += analogRead(LDR_PIN);
    }
    return sum / SAMPLES;
}

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    for (int i = 0; i < 20; i++) {
        analogRead(LDR_PIN);
        delay(2);
    }

    rawMin = rawMax = readRawAveraged();
}

void loop() {
    if (pressed(BUTTON_PIN, buttonLastReading, buttonState, buttonDebounceTime)) {
        swapped = !swapped;
    }

    int raw = readRawAveraged();
    rawMin = min(rawMin, raw);
    rawMax = max(rawMax, raw);

    int midpoint = (rawMin + rawMax) / 2;
    int hysteresis = max((rawMax - rawMin) / 20, 1);

    if (raw < midpoint - hysteresis) isBright = true;
    else if (raw > midpoint + hysteresis) isBright = false;

    int lightDuty = isBright ? 255 : 0;
    int darkDuty = 255 - lightDuty;

    analogWrite(RED_LED_PIN, swapped ? darkDuty : lightDuty);
    analogWrite(GREEN_LED_PIN, swapped ? lightDuty : darkDuty);

    Serial.printf("raw=%d light=%d dark=%d swapped=%d min=%d max=%d\n", raw, lightDuty, darkDuty, swapped, rawMin, rawMax);
    delay(20);
}