#include "PressureSensor.h"
#include "DebugHelper.h"

#if ARDUINO_ARCH_ESP32
#include "esp32-hal-adc.h"
#endif

// 构造函数：初始化压力传感器对象
PressureSensor::PressureSensor(uint8_t pin) : _pin(pin), _initialized(false), _bufferIndex(0) {
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
        _adcBuffer[i] = 0;
    }
}

void PressureSensor::begin() {
#if PRESSURE_SIGNAL_ANALOG
    // 纯模拟节点：不要内部上拉，以免与 10k 分压并联拉偏电压
    pinMode(_pin, INPUT);
#else
    pinMode(_pin, INPUT_PULLUP);
#endif

#if ARDUINO_ARCH_ESP32
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
#endif

    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
        _adcBuffer[i] = analogRead(_pin);
        delay(10);
    }

    _initialized = true;
    DEBUG_INFO(DEBUG_MODULE_SENSOR, "Pressure sensor initialized on GPIO%d", _pin);

    int raw = analogRead(_pin);
#if PRESSURE_SIGNAL_ANALOG
    Serial.printf("[PRESSURE] 模拟模式 GPIO%d: analog_raw=%d (不按应偏大，按下应变小)\n", _pin, raw);
#else
    Serial.printf("[PRESSURE] 比较器模式 GPIO%d: analog_raw=%d digital=%s\n",
                  _pin, raw, digitalRead(_pin) ? "HIGH" : "LOW");
#endif
}

int PressureSensor::getFilteredADC() {
    if (!_initialized) {
        DEBUG_ERROR(DEBUG_MODULE_SENSOR, "Pressure sensor not initialized");
        return 0;
    }

    int adcValue = analogRead(_pin);

    _adcBuffer[_bufferIndex] = adcValue;
    _bufferIndex = (_bufferIndex + 1) % FILTER_WINDOW_SIZE;

    return calculateMovingAverage();
}

int PressureSensor::calculateMovingAverage() {
    long sum = 0;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
        sum += _adcBuffer[i];
    }
    return sum / FILTER_WINDOW_SIZE;
}

bool PressureSensor::isPressureDetected() {
    if (!_initialized) {
        DEBUG_ERROR(DEBUG_MODULE_SENSOR, "Pressure sensor not initialized");
        return false;
    }

#if PRESSURE_SIGNAL_ANALOG
    // 电压从不按(高)到按下(低)，ADC 减小
    return (int)analogRead(_pin) < PRESSURE_ADC_PRESS_BELOW;
#else
#if PRESSURE_ACTIVE_LOW
    return digitalRead(_pin) == LOW;
#else
    return digitalRead(_pin) == HIGH;
#endif
#endif
}

bool PressureSensor::isInitialized() const {
    return _initialized;
}
