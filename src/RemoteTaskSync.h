#ifndef REMOTE_TASK_SYNC_H
#define REMOTE_TASK_SYNC_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// API 配置常量
#define API_BASE_URL "https://storytoy.freefly-ai.com"
#define API_TASK_LIST_ENDPOINT "/api/v1/deviceControl/query"  // 正确的拉取接口
#define API_TASK_UPDATE_ENDPOINT "/api/v1/task/update"  // 保留，但可能不使用
#define API_TASK_ACK_ENDPOINT "/api/v1/deviceControl/ack"  // 任务确认接口

// API Token 配置（通过 platformio.ini 的 build_flags 配置）
// 请在 platformio.ini 中修改：-D API_TOKEN_CONFIGURED="您的实际 Token"
#ifndef API_TOKEN_CONFIGURED
  #error "API_TOKEN_CONFIGURED 未定义！请在 platformio.ini 的 build_flags 中配置 -D API_TOKEN_CONFIGURED=\"YOUR_TOKEN\""
#endif
#define API_TOKEN API_TOKEN_CONFIGURED

#define POLLING_INTERVAL_MS 60000    // 60 秒轮询间隔

// Flash 存储配置
#define STORAGE_NAMESPACE "alarm_data"
#define STORAGE_KEY_TIME "alarm_time"
#define STORAGE_KEY_TASK_ID "task_id"

// 远程任务同步类
class RemoteTaskSync {
public:
    // 初始化远程任务同步
    static void init(const char* deviceId);
    
    // 主循环轮询函数（非阻塞）
    static void loop();
    
    // 获取任务列表
    static bool fetchTasks();
    
    // 保存闹钟时间到 Flash
    static bool saveAlarmToFlash(String time, String taskId);
    
    // 确认任务成功
    static bool confirmTaskSuccess(String taskId);
    
    // 获取上次同步时间
    static unsigned long getLastSyncTime();
    
    // 获取存储的闹钟时间
    static String getStoredAlarmTime();
    
    // 获取存储的任务 ID
    static String getStoredTaskId();

private:
    static String m_deviceId;                      // 设备 ID
    static unsigned long m_lastSyncTime;           // 上次同步时间
    static unsigned long m_lastPollTime;           // 上次轮询时间
    static bool m_isInitialized;                   // 是否已初始化
    static WiFiClientSecure m_wifiClient;          // WiFi 客户端
};

#endif
