#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedBlinker {
public:
    LedBlinker(uint8_t pin, uint16_t numLeds = 1);
    
    void begin();
    void update();
    void setInterval(uint32_t intervalMs);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setBrightness(uint8_t brightness);
    void turnOn();
    void turnOff();
    void toggle();
    bool isOn() const;

private:
    Adafruit_NeoPixel _strip;
    uint32_t _intervalMs;
    uint32_t _lastToggleTime;
    bool _state;
    uint8_t _r, _g, _b;
    uint8_t _brightness;
};
