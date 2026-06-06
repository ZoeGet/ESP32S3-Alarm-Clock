#include "Timer.h"

// 构造函数：初始化定时器对象
Timer::Timer() : nextUpdateTime(0) {}

// 初始化定时器：设置下一次更新时间
void Timer::begin() {
    nextUpdateTime = millis() + UPDATE_INTERVAL;
}

// 等待下一个更新周期：固定主循环节拍（见 Timer.h 中的 UPDATE_INTERVAL）
void Timer::waitForNextCycle() {
    unsigned long currentTime = millis();
    unsigned long waitTime = nextUpdateTime - currentTime;
    
    // 计算需要等待的时间
    if (waitTime > 0 && waitTime <= UPDATE_INTERVAL) {
        delay(waitTime);
    }
    
    // 更新下一次更新时间
    nextUpdateTime += UPDATE_INTERVAL;
}
