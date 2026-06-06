#ifndef ALARM_TRIGGER_H
#define ALARM_TRIGGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "Clock.h"
#include "Alarm.h"

// 存储配置
#define ALARM_STORAGE_NAMESPACE "alarm_config"
#define ALARM_STORAGE_KEY_ALARMS "alarms"  // 存储所有闹钟的 JSON 数据

// 单个闹钟数据结构
struct AlarmData {
    String taskId;      // 任务 ID
    String time;        // 闹钟时间 (HH:mm)
    String fileId;      // 文件 ID
    String audioUrl;    // 音频文件 URL（从服务器 query 接口返回）
    String alarmId;     // 闹钟 ID（用于取消）
};

// 闹钟触发回调函数类型
typedef void (*AlarmTriggerCallback)();

// 闹钟触发器类
class AlarmTrigger {
public:
    // 初始化闹钟触发器
    static void init();
    
    // 设置闹钟触发回调
    static void setAlarmCallback(AlarmTriggerCallback callback);
    
    // 处理闹钟任务（数据解析与存储）
    static bool processAlarmTask(String jsonResponse);
    
    // 处理取消闹钟任务
    static bool processAlarmCancel(String taskId, String alarmId);
    
    // 检查并触发闹钟（本地比对与触发）
    static void checkAndTriggerAlarm();
    
    // 获取所有闹钟
    static JsonArray getAllAlarms();
    
    // 清除单个闹钟
    static bool removeAlarm(String taskId);
    
    // 清除所有闹钟
    static void clearAllAlarms();
    
    // 获取指定 task_id 的 audio_url
    static String getAudioUrlByTaskId(String taskId);
    
    // 获取当前触发的 task_id
    static String getCurrentTriggeredTaskId();

private:
    static bool m_isAlarmed;              // 触发锁标志位
    static String m_lastAlarmTime;        // 上次触发闹钟的时间（用于分钟跳转检测）
    static AlarmTriggerCallback m_alarmCallback;  // 闹钟触发回调
};

#endif
