#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48      // LED引脚

// LED类：控制NeoPixel LED灯
class LED {
public:
    // 构造函数：初始化LED对象
    LED(uint8_t pin, uint16_t numLeds = 1);
    
    // 初始化LED
    void begin();
    
    // 更新LED状态
    void update();
    
    // 设置闪烁间隔
    void setInterval(uint32_t intervalMs);
    
    // 设置LED颜色
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    
    // 设置亮度
    void setBrightness(uint8_t brightness);
    
    // 打开LED
    void turnOn();
    
    // 关闭LED
    void turnOff();
    
    // 切换LED状态
    void toggle();
    
    // 检查LED是否开启
    bool isOn() const;

private:
    Adafruit_NeoPixel _strip;    // NeoPixel对象
    uint32_t _intervalMs;         // 闪烁间隔
    uint32_t _lastToggleTime;     // 上次切换时间
    bool _state;                  // LED状态
    uint8_t _r;                   // 红色值
    uint8_t _g;                   // 绿色值
    uint8_t _b;                   // 蓝色值
    uint8_t _brightness;          // 亮度
};

#endif // LED_H
