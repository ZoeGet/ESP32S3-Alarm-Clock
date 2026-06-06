#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Arduino.h>

// 压力传感器引脚定义（原理图 PRESSURE → IO4）
#define PRESSURE_SENSOR_PIN 4

// ---------- 信号来源（二选一）----------
// 1：GPIO4 飞线接到「薄膜分压点」（测得不按~3.3V、按下~2.4V），走 ADC，不依赖 LM393 输出。
// 0：GPIO4 仍接 LM393 数字输出（开漏），用 digitalRead + 内部上拉。
#define PRESSURE_SIGNAL_ANALOG 1

#if PRESSURE_SIGNAL_ANALOG
// 瞬时 ADC 小于此值判为「按下」（触发蜂鸣 / 停播等）。可按实测微调。
#define PRESSURE_ADC_PRESS_BELOW 2400
#else
#define PRESSURE_ACTIVE_LOW 1  // 1：输出拉低为触发；0：高为触发
#endif

// 滑动窗口越小屏显跟踪越快（主循环约 40ms 时，窗口 4 ≈ 160ms 内跟上阶跃）
#define FILTER_WINDOW_SIZE 4

// 压力传感器类：处理压力传感器数据采集和阈值判断
class PressureSensor {
public:
    PressureSensor(uint8_t pin = PRESSURE_SENSOR_PIN);  // 构造函数：初始化压力传感器对象

    void begin();  // 初始化压力传感器

    // 读取滤波后的 ADC 值（OLED 显示）
    int getFilteredADC();

    // 检查是否达到压力阈值
    bool isPressureDetected();

    // 检查是否已初始化
    bool isInitialized() const;

private:
    uint8_t _pin;                  // 压力传感器引脚
    bool _initialized;             // 初始化状态
    int _adcBuffer[FILTER_WINDOW_SIZE];  // ADC 值缓冲区
    int _bufferIndex;              // 缓冲区索引

    // 计算滑动平均值
    int calculateMovingAverage();
};

#endif // PRESSURE_SENSOR_H
