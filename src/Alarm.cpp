#include "Alarm.h"
#include "DebugHelper.h"

/**
 * 构造函数
 * 初始化闹钟对象，设置默认值
 * @param speaker 喇叭对象引用
 * @param display OLED显示对象引用
 */
Alarm::Alarm(Speaker &speaker, OLED_Display &display) : _speaker(speaker), 
    _display(display), _alarmHour(0), _alarmMinute(0), _alarmSecond(0), 
    _lastTriggerTime(0) {}

/**
 * 设置闹钟时间
 * 使用取模运算确保时间值在有效范围内
 * @param hour 小时（0-23）
 * @param minute 分钟（0-59）
 * @param second 秒（0-59）
 */
void Alarm::setAlarmTime(uint8_t hour, uint8_t minute, uint8_t second) {
    // 验证时间范围，确保值在有效范围内
    _alarmHour = hour % 24;
    _alarmMinute = minute % 60;
    _alarmSecond = second % 60;
    
    // 打印设置的闹钟时间到串口
    DEBUG_INFO(DEBUG_MODULE_ALARM, "Alarm set to %02d:%02d:%02d", _alarmHour, _alarmMinute, _alarmSecond);
}

/**
 * 检查闹钟实现
 * 1. 检查当前时间是否到达闹钟时间
 * 2. 检查是否在最小触发间隔内
 * 3. 如果条件满足，触发闹钟
 * @param now 当前时间对象
 */
void Alarm::checkAlarm(const DateTime &now) {
    if (isAlarmTime(now)) {
        // 检查触发间隔，防止短时间内重复触发
        unsigned long currentTime = millis();
        if (currentTime - _lastTriggerTime >= _minTriggerInterval) {
            triggerAlarm();
            _lastTriggerTime = currentTime;
        }
    }
}

/**
 * 获取闹钟时间
 * 将当前设置的闹钟时间赋值给输出参数
 * @param hour 输出参数，存储小时
 * @param minute 输出参数，存储分钟
 * @param second 输出参数，存储秒
 */
void Alarm::getAlarmTime(uint8_t &hour, uint8_t &minute, uint8_t &second) const {
    hour = _alarmHour;
    minute = _alarmMinute;
    second = _alarmSecond;
}

/**
 * 触发闹钟
 * 1. 播放1kHz声音，持续100ms
 * 2. 在OLED屏幕上显示提醒信息
 * 3. 打印触发信息到串口
 */
void Alarm::triggerAlarm() {
    // 播放 1kHz 声音，持续 100ms
    _speaker.playTone(1000, 100);
    // 显示闹钟触发信息
    _display.showAlarmTriggered();
    // 打印触发信息到串口
    DEBUG_INFO(DEBUG_MODULE_ALARM, "Alarm triggered!");
}

/**
 * 检查是否到达闹钟时间
 * 比较当前时间与设置的闹钟时间
 * @param now 当前时间对象
 * @return 是否到达闹钟时间
 */
bool Alarm::isAlarmTime(const DateTime &now) const {
    return now.hour() == _alarmHour && 
           now.minute() == _alarmMinute && 
           now.second() == _alarmSecond;
}

/**
 * 播放闹钟音乐（用于远程闹钟触发）
 * 播放 30 秒的闹钟音乐
 */
void Alarm::playAlarmMusic() {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "Playing alarm music for 30 seconds...");
    // 播放 30 秒闹钟音乐（实际播放逻辑在 Speaker 中）
    // 这里调用 playOneSecondTone 30 次作为示例
    for (int i = 0; i < 30; i++) {
        _speaker.playOneSecondTone();
        delay(1000);
    }
    DEBUG_INFO(DEBUG_MODULE_ALARM, "Alarm music finished");
}
