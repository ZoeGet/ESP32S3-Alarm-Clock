#ifndef ALARM_H
#define ALARM_H

#include <RTClib.h>
#include "Speaker.h"
#include "OLED_Display.h"

// 闹钟类：管理闹钟时间设置、检测时间到达、触发声音提醒
class Alarm {
public:
    // 构造函数：初始化闹钟对象
    Alarm(Speaker &speaker, OLED_Display &display);
    
    // 设置闹钟时间：小时（0-23）、分钟（0-59）、秒（0-59）
    void setAlarmTime(uint8_t hour, uint8_t minute, uint8_t second);
    
    // 检查当前时间是否到达闹钟时间
    void checkAlarm(const DateTime &now);
    
    // 获取当前设置的闹钟时间
    void getAlarmTime(uint8_t &hour, uint8_t &minute, uint8_t &second) const;
    
    // 播放闹钟音乐（用于远程闹钟触发）
    void playAlarmMusic();
    
private:
    Speaker &_speaker;          // 喇叭对象引用
    OLED_Display &_display;     // OLED显示对象引用
    uint8_t _alarmHour;         // 闹钟小时
    uint8_t _alarmMinute;       // 闹钟分钟
    uint8_t _alarmSecond;       // 闹钟秒
    unsigned long _lastTriggerTime;  // 上次触发时间
    const unsigned long _minTriggerInterval = 60000;  // 1分钟最小触发间隔
    
    // 触发闹钟：播放声音并显示提醒信息
    void triggerAlarm();
    
    // 检查当前时间是否为闹钟时间
    bool isAlarmTime(const DateTime &now) const;
};

#endif // ALARM_H
