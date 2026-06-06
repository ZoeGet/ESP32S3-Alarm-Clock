#include "LED.h"

LED::LED(uint8_t pin, uint16_t numLeds)
    : _strip(numLeds, pin, NEO_GRB + NEO_KHZ800)
    , _intervalMs(500)
    , _lastToggleTime(0)
    , _state(false)
    , _r(0)
    , _g(0)
    , _b(255)
    , _brightness(25)
{
}

void LED::begin() {
    _strip.begin();
    _strip.show();
    _strip.setBrightness(_brightness);
    _lastToggleTime = millis();  // 记录开始时间
    turnOn();  // 先点亮
}

void LED::update() {
    uint32_t currentTime = millis();
    
    if (currentTime - _lastToggleTime >= _intervalMs) {
        toggle();
        _lastToggleTime = currentTime;
    }
}

void LED::setInterval(uint32_t intervalMs) {
    _intervalMs = intervalMs;
}

void LED::setColor(uint8_t r, uint8_t g, uint8_t b) {
    _r = r;
    _g = g;
    _b = b;
    if (_state) {
        turnOn();
    }
}

void LED::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    _strip.setBrightness(_brightness);
    if (_state) {
        turnOn();
    }
}

void LED::turnOn() {
    _state = true;
    _strip.setPixelColor(0, _strip.Color(_r, _g, _b));
    _strip.show();
}

void LED::turnOff() {
    _state = false;
    _strip.setPixelColor(0, _strip.Color(0, 0, 0));
    _strip.show();
}

void LED::toggle() {
    if (_state) {
        turnOff();
    } else {
        turnOn();
    }
}

bool LED::isOn() const {
    return _state;
}
