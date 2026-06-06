#include "RemoteTaskSync.h"
#include "AlarmTrigger.h"
#include "DebugHelper.h"

// 静态成员初始化
String RemoteTaskSync::m_deviceId = "";
unsigned long RemoteTaskSync::m_lastSyncTime = 0;
unsigned long RemoteTaskSync::m_lastPollTime = 0;
bool RemoteTaskSync::m_isInitialized = false;
WiFiClientSecure RemoteTaskSync::m_wifiClient;

void RemoteTaskSync::init(const char* deviceId) {
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 初始化远程任务同步模块...");
    
    // 处理设备 ID：去除首尾空格并转换为大写
    m_deviceId = String(deviceId);
    m_deviceId.trim();           // 去除首尾空格
    m_deviceId.toUpperCase();    // 转换为大写
    m_lastSyncTime = 0;
    m_lastPollTime = 0;
    m_isInitialized = true;
    
    // 设置 SSL 证书校验（忽略证书）
    m_wifiClient.setInsecure();
    
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 初始化完成");
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 原始设备 ID: %s", String(deviceId).c_str());
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 处理后设备 ID: %s", m_deviceId.c_str());
}

void RemoteTaskSync::loop() {
    if (!m_isInitialized) {
        DEBUG_ERROR(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 错误：模块未初始化");
        return;
    }
    
    // 检查 WiFi 连接状态
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] WiFi 未连接，跳过任务同步");
        return;
    }
    
    // 非阻塞定时器：检查是否到达轮询间隔
    unsigned long currentMillis = millis();
    if (currentMillis - m_lastPollTime >= POLLING_INTERVAL_MS) {
        m_lastPollTime = currentMillis;
        
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] === 开始任务同步 ===");
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 距离上次同步：%lu 秒", (currentMillis - m_lastSyncTime) / 1000);
        
        // 执行任务同步
        if (fetchTasks()) {
            m_lastSyncTime = currentMillis;
        }
        
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] === 任务同步结束 ===");
    }
}

bool RemoteTaskSync::fetchTasks() {
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] >> 获取任务列表...");
    
    // 确保设备 ID 是干净的（再次处理）
    String deviceId = m_deviceId;
    deviceId.trim();
    deviceId.toUpperCase();
    
    // 构建请求 URL
    String url = String(API_BASE_URL) + String(API_TASK_LIST_ENDPOINT) + 
                 "?device_id=" + deviceId;
    
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 请求 URL: %s", url.c_str());
    
    // 创建 HTTP 客户端
    HTTPClient http;
    
    // 发起 GET 请求
    http.begin(m_wifiClient, url);
    
    // 添加 Authorization 头（关键！）
    String authHeader = "Bearer " + String(API_TOKEN);
    http.addHeader("Authorization", authHeader);
    http.addHeader("Content-Type", "application/json");
    
    // 打印最终请求 URL 用于调试
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "最终请求的 URL: %s", url.c_str());
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "Authorization Header: Bearer %s...", String(API_TOKEN).substring(0, 20).c_str());
    
    int httpResponseCode = http.GET();
    
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] HTTP 响应码：%d", httpResponseCode);
    
    if (httpResponseCode != 200) {
        DEBUG_ERROR(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 错误：HTTP 请求失败，响应码：%d", httpResponseCode);
        http.end();
        return false;
    }
    
    // 读取响应内容
    String responsePayload = http.getString();
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 响应内容：%s", responsePayload.c_str());
    
    http.end();
    
    // 解析 JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, responsePayload);
    
    if (error) {
        DEBUG_ERROR(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] JSON 解析错误：%s", error.c_str());
        return false;
    }
    
    if (!doc["data"].is<JsonObject>() || !doc["data"]["commands"].is<JsonArray>()) {
        return false;
    }
    
    // 获取 commands 数组
    JsonArray commands = doc["data"]["commands"].as<JsonArray>();
    
    if (commands.size() == 0) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 没有任务");
        return true;
    }
    
    // 遍历任务列表，处理 alarm_set 和 alarm_cancel 类型
    bool foundAlarmTask = false;
    bool foundAlarmCancel = false;
    String taskType = "";
    String taskId = "";
    String alarmTime = "";
    
    for (JsonObject command : commands) {
        taskType = command["command_type"] | "";  // 注意：服务器返回的是 command_type 不是 task_type
        
        if (taskType == "alarm_set") {
            taskId = command["id"] | "";  // 注意：服务器返回的是 id 不是 task_id
            
            // 注意：服务器返回的是 params 不是 payload
            if (command["params"].is<JsonObject>() && command["params"]["time"].is<const char*>()) {
                alarmTime = command["params"]["time"] | "";
            }
            
            DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 找到 alarm_set 任务：task_id=%s, time=%s", taskId.c_str(), alarmTime.c_str());
            
            if (taskId.length() > 0 && alarmTime.length() > 0) {
                foundAlarmTask = true;
                
                // 使用 AlarmTrigger 处理数据（包含防重处理和存储）
                String jsonStr;
                serializeJson(doc, jsonStr);
                if (AlarmTrigger::processAlarmTask(jsonStr)) {
                    // 发送确认
                    confirmTaskSuccess(taskId);
                }
                
                break;  // 只处理第一个 alarm_set 任务
            }
        } else if (taskType == "alarm_cancel") {
            taskId = command["id"] | "";
            String alarmId = "";
            
            // 提取 alarm_id
            if (command["params"].is<JsonObject>() && command["params"]["alarm_id"].is<const char*>()) {
                alarmId = command["params"]["alarm_id"] | "";
            }
            
            DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 找到 alarm_cancel 任务：task_id=%s, alarm_id=%s", taskId.c_str(), alarmId.c_str());
            
            if (taskId.length() > 0) {
                foundAlarmCancel = true;
                
                // 使用 AlarmTrigger 处理取消闹钟（传递 task_id 和 alarm_id）
                if (AlarmTrigger::processAlarmCancel(taskId, alarmId)) {
                    // 发送确认
                    confirmTaskSuccess(taskId);
                }
            }
        }
    }
    
    if (!foundAlarmTask && !foundAlarmCancel) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 未找到 alarm_set 或 alarm_cancel 类型的任务");
    }
    
    return true;
}

bool RemoteTaskSync::saveAlarmToFlash(String time, String taskId) {
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] >> 保存闹钟时间到 Flash...");
    
    // 检查数据是否已存在
    String storedTime = getStoredAlarmTime();
    String storedTaskId = getStoredTaskId();
    
    if (storedTime == time && storedTaskId == taskId) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 数据未变化，跳过存储");
        return true;
    }
    
    // 初始化 Preferences
    Preferences preferences;
    bool opened = preferences.begin(STORAGE_NAMESPACE, false);
    
    if (!opened) {
        DEBUG_ERROR(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 错误：无法打开命名空间");
        return false;
    }
    
    // 写入数据
    bool timeSaved = preferences.putString(STORAGE_KEY_TIME, time);
    bool taskIdSaved = preferences.putString(STORAGE_KEY_TASK_ID, taskId);
    
    preferences.end();
    
    if (timeSaved && taskIdSaved) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 存储成功：time=%s, task_id=%s", time.c_str(), taskId.c_str());
        return true;
    } else {
        DEBUG_ERROR(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 错误：存储失败");
        return false;
    }
}

bool RemoteTaskSync::confirmTaskSuccess(String taskId) {
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] >> 发送任务确认...");
    
    // 检查 WiFi 连接
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] WiFi 未连接，跳过确认");
        return false;
    }
    
    // 构建请求 URL - 使用服务器管理员确认的接口
    String url = String(API_BASE_URL) + String(API_TASK_ACK_ENDPOINT);
    
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 确认 URL: %s", url.c_str());
    
    // 创建 HTTP 客户端
    HTTPClient http;
    
    // 发起 POST 请求
    http.begin(m_wifiClient, url);
    // 注意：服务器管理员说这个接口不需要 Authorization 头
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;
    
    doc["device_id"] = m_deviceId;
    doc["timestamp"] = time(nullptr);
    
    JsonArray commandIds = doc["command_ids"].to<JsonArray>();
    commandIds.add(taskId);
    
    String requestBody;
    serializeJson(doc, requestBody);
    
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 请求体：%s", requestBody.c_str());
    
    int httpResponseCode = http.POST(requestBody);
    
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] HTTP 响应码：%d", httpResponseCode);
    
    if (httpResponseCode == 200 || httpResponseCode == 201) {
        String response = http.getString();
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 确认成功：%s", response.c_str());
        http.end();
        return true;
    } else {
        DEBUG_ERROR(DEBUG_MODULE_NETWORK, "[RemoteTaskSync] 错误：确认失败，响应码：%d", httpResponseCode);
        http.end();
        return false;
    }
}

unsigned long RemoteTaskSync::getLastSyncTime() {
    return m_lastSyncTime;
}

String RemoteTaskSync::getStoredAlarmTime() {
    Preferences preferences;
    bool opened = preferences.begin(STORAGE_NAMESPACE, false);
    
    if (!opened) {
        return "";
    }
    
    String time = preferences.getString(STORAGE_KEY_TIME, "");
    preferences.end();
    
    return time;
}

String RemoteTaskSync::getStoredTaskId() {
    Preferences preferences;
    bool opened = preferences.begin(STORAGE_NAMESPACE, false);
    
    if (!opened) {
        return "";
    }
    
    String taskId = preferences.getString(STORAGE_KEY_TASK_ID, "");
    preferences.end();
    
    return taskId;
}
