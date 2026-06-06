#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

// 定时器类：用于控制系统循环更新间隔
class Timer {
public:
    // 构造函数：初始化定时器对象
    Timer();
    
    // 初始化定时器
    void begin();
    
    // 等待下一个更新周期（用于统一 pacing；闹钟仍按 RTC 分钟比对）
    void waitForNextCycle();
    
private:
    unsigned long nextUpdateTime;  // 下一次更新时间
    // 压力模拟 ADC + OLED 需较快刷新；原先 1000ms 会导致滑动平均约「窗口×1秒」才回落
    static const unsigned long UPDATE_INTERVAL = 25; // ms，约 30Hz 主循环
};

#endif // TIMER_H
