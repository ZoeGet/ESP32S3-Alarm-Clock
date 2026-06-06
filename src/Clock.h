#ifndef CLOCK_H
#define CLOCK_H

#include <RTClib.h>
#include <WiFiManager.h>
#include "OLED_Display.h"

// 时钟类：管理系统时间，支持从NTP服务器同步时间和从编译时间获取初始时间
class Clock {
public:
    Clock(); // 构造函数：初始化时钟对象

    // 初始化时钟：尝试从NTP服务器同步时间，如果失败则使用编译时间
    void begin(OLED_Display &display);
    
    // 获取当前时间
    DateTime now();

private:
    RTC_Millis rtc;         // 实时时钟对象
    bool initialized;       // 初始化状态标志
    bool ntpSynced;         // NTP是否同步成功

    // 解析编译日期：从__DATE__宏中解析年、月、日
    void parseCompileDate(int &year, int &month, int &day);
    
    // 解析编译时间：从__TIME__宏中解析时、分、秒
    void parseCompileTime(int &hour, int &minute, int &second);
    
    // 从编译时间设置系统时间：当NTP同步失败时使用
    void setTimeFromCompileTime();
    
    // 从NTP服务器同步时间：返回同步是否成功
    bool setTimeFromNTP(OLED_Display &display);
};

#endif // CLOCK_H
