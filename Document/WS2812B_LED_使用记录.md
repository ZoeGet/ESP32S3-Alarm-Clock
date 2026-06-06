# WS2812B LED 使用记录

## 硬件信息

- **LED 型号**: WS2812B（RGB 可寻址 LED）
- **连接引脚**: GPIO48（ESP32-S3-DevKitC-1 板载 RGB LED）
- **控制方式**: 单线串行通信，需要专用库驱动

## 关键教训

### ❌ 错误尝试

**第一次**: 使用普通 GPIO 控制
```cpp
// 错误！WS2812B 不是普通 LED，不能用 digitalWrite
pinMode(48, OUTPUT);
digitalWrite(48, HIGH);  // 无效！
```

**结果**: LED 无反应

**原因**: WS2812B 是智能 LED，需要特定的时序协议（单线串行通信），不是简单的开关控制。

---

### ✅ 正确方案

**使用 Adafruit NeoPixel 库**

#### 1. 添加库依赖 (platformio.ini)
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps = 
    adafruit/Adafruit NeoPixel@^1.12.0
```

#### 2. 封装类设计 (LedBlinker.h / LedBlinker.cpp)

**LedBlinker.h**
```cpp
#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedBlinker {
public:
    LedBlinker(uint8_t pin, uint16_t numLeds = 1);
    
    void begin();
    void update();                    // 非阻塞闪烁更新
    void setInterval(uint32_t intervalMs);
    void setColor(uint8_t r, uint8_t g, uint8_t b);  // RGB 颜色
    void setBrightness(uint8_t brightness);          // 亮度 0-255
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
```

**LedBlinker.cpp 关键点**
```cpp
// 初始化时指定 LED 类型为 NEO_GRB + NEO_KHZ800
LedBlinker::LedBlinker(uint8_t pin, uint16_t numLeds)
    : _strip(numLeds, pin, NEO_GRB + NEO_KHZ800)  // 关键！
    , _intervalMs(500)
    , _lastToggleTime(0)
    , _state(false)
    , _r(0), _g(0), _b(255)    // 默认蓝色
    , _brightness(20)           // 默认亮度 20
{
}

void LedBlinker::begin() {
    _strip.begin();
    _strip.show();              // 初始化显示
    _strip.setBrightness(_brightness);
    turnOff();
}

void LedBlinker::turnOn() {
    _state = true;
    _strip.setPixelColor(0, _strip.Color(_r, _g, _b));
    _strip.show();              // 必须调用 show() 才生效！
}
```

#### 3. 使用示例 (main.cpp)

```cpp
#include <Arduino.h>
#include "LedBlinker.h"

constexpr uint8_t LED_PIN = 48;
constexpr uint32_t BLINK_INTERVAL_MS = 500;

LedBlinker led(LED_PIN);

void setup() {
    led.begin();
    led.setInterval(BLINK_INTERVAL_MS);
    led.setColor(0, 0, 255);      // 蓝色 (R, G, B)
    led.setBrightness(20);         // 亮度 20/255（较暗）
}

void loop() {
    led.update();  // 非阻塞闪烁
}
```

## 重要注意事项

### 1. LED 类型标识
| 标识 | 含义 |
|------|------|
| `NEO_GRB` | 颜色顺序：绿-红-蓝 |
| `NEO_KHZ800` | 800KHz 通信速率（WS2812B 标准）|

### 2. 必须调用 show()
设置颜色后必须调用 `_strip.show()`，否则 LED 不会更新！

### 3. 亮度设置
- 范围：0-255
- 建议值：10-50（太亮刺眼）
- 可在运行时动态调整

### 4. 颜色格式
```cpp
setColor(255, 0, 0);   // 红色
setColor(0, 255, 0);   // 绿色
setColor(0, 0, 255);   // 蓝色
setColor(255, 255, 0); // 黄色
setColor(255, 0, 255); // 紫色
setColor(255, 255, 255); // 白色
```

## 文件结构

```
project/
├── include/
│   └── LedBlinker.h          # LED 类头文件
├── src/
│   ├── main.cpp              # 主程序
│   └── LedBlinker.cpp        # LED 类实现
└── platformio.ini            # 库依赖配置
```

## 总结

| 项目 | 说明 |
|------|------|
| LED 类型 | WS2812B 智能 RGB LED |
| 控制方式 | 单线串行，需专用库 |
| 推荐库 | Adafruit NeoPixel |
| 关键方法 | `begin()`, `setPixelColor()`, `show()` |
| 注意事项 | 必须调用 `show()` 才能更新显示 |

---

**记录时间**: 2026-02-26  
**开发板**: ESP32-S3-DevKitC-1  
**LED 位置**: GPIO48（板载 WS2812B）
