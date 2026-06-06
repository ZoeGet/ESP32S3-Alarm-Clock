#include "AlarmTrigger.h"
#include "DebugHelper.h"

// 静态成员初始化
bool AlarmTrigger::m_isAlarmed = false;
String AlarmTrigger::m_lastAlarmTime = "";
AlarmTriggerCallback AlarmTrigger::m_alarmCallback = nullptr;

// 触发状态跟踪（每个闹钟独立）
static String m_lastTriggeredTaskId = "";

void AlarmTrigger::init() {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 初始化闹钟触发器模块...");
    m_isAlarmed = false;
    m_lastAlarmTime = "";
    m_alarmCallback = nullptr;
    m_lastTriggeredTaskId = "";
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 初始化完成");
}

void AlarmTrigger::setAlarmCallback(AlarmTriggerCallback callback) {
    m_alarmCallback = callback;
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 闹钟回调已设置");
}

// 辅助函数：从 Flash 读取所有闹钟
static JsonArray readAlarmsFromFlash() {
    Preferences preferences;
    bool opened = preferences.begin(ALARM_STORAGE_NAMESPACE, false);
    
    if (!opened) {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：无法打开命名空间");
        static StaticJsonDocument<0> emptyDoc;
        return emptyDoc.to<JsonArray>();
    }
    
    String alarmsJson = preferences.getString(ALARM_STORAGE_KEY_ALARMS, "[]");
    preferences.end();
    
    static StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);
    
    if (error) {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] JSON 解析错误：%s", error.c_str());
        static StaticJsonDocument<0> emptyDoc;
        return emptyDoc.to<JsonArray>();
    }
    
    return doc.as<JsonArray>();
}

// 辅助函数：保存所有闹钟到 Flash
static bool writeAlarmsToFlash(JsonArray alarms) {
    Preferences preferences;
    bool opened = preferences.begin(ALARM_STORAGE_NAMESPACE, false);
    
    if (!opened) {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：无法打开命名空间");
        return false;
    }
    
    String alarmsJson;
    serializeJson(alarms, alarmsJson);
    
    bool saved = preferences.putString(ALARM_STORAGE_KEY_ALARMS, alarmsJson);
    preferences.end();
    
    if (saved) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 闹钟列表已保存到 Flash: %s", alarmsJson.c_str());
    } else {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：保存失败");
    }
    
    return saved;
}

bool AlarmTrigger::processAlarmTask(String jsonResponse) {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] >> 处理闹钟任务...");
    
    // 解析 JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error) {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] JSON 解析错误：%s", error.c_str());
        return false;
    }
    
    // 检查响应结构：{"code": 0, "message": "success", "data": {"commands": [...]}}
    if (!doc["data"].is<JsonObject>() || !doc["data"]["commands"].is<JsonArray>()) {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：响应格式不正确，缺少 data.commands");
        return false;
    }
    
    // 获取 commands 数组
    JsonArray commands = doc["data"]["commands"].as<JsonArray>();
    
    if (commands.size() == 0) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 没有任务");
        return true;
    }
    
    // 读取当前所有闹钟
    JsonArray currentAlarms = readAlarmsFromFlash();
    
    // 遍历任务列表，查找 alarm_set 类型
    bool foundAlarmTask = false;
    
    for (JsonObject command : commands) {
        String taskType = command["command_type"] | "";
        
        if (taskType == "alarm_set") {
            String taskId = command["id"] | "";
            String alarmTime = "";
            String fileId = "";
            String audioUrl = "";
            String alarmId = "";
            
            // 提取 time
            if (command["params"].is<JsonObject>() && command["params"]["time"].is<const char*>()) {
                alarmTime = command["params"]["time"] | "";
            }
            
            // 提取 file_id
            if (command["params"]["file_id"].is<const char*>()) {
                fileId = command["params"]["file_id"] | "";
            }
            
            // 提取 audio_url（后端文档步骤 5：从 params.audio_url 获取）
            if (command["params"]["audio_url"].is<const char*>()) {
                audioUrl = command["params"]["audio_url"] | "";
            }
            
            // 提取 alarm_id（如果有）
            if (command["params"]["alarm_id"].is<const char*>()) {
                alarmId = command["params"]["alarm_id"] | "";
            }
            
            Serial.print("[AlarmTrigger] 找到 alarm_set 任务：");
            Serial.print("task_id=");
            Serial.print(taskId);
            Serial.print(", time=");
            Serial.print(alarmTime);
            Serial.print(", alarm_id=");
            Serial.println(alarmId);
            
            if (taskId.length() > 0 && alarmTime.length() > 0) {
                foundAlarmTask = true;
                
                // 检查是否已存在相同 task_id
                bool alreadyExists = false;
                for (JsonObject alarm : currentAlarms) {
                    if (String(alarm["taskId"] | "") == taskId) {
                        alreadyExists = true;
                        Serial.print("[AlarmTrigger] 任务已存在，跳过：");
                        Serial.println(taskId);
                        break;
                    }
                }
                
                if (alreadyExists) {
                    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 任务已存在，跳过：%s", taskId.c_str());
                    continue;
                }
                
                // 添加到闹钟列表
                JsonObject newAlarm = currentAlarms.add<JsonObject>();
                newAlarm["taskId"] = taskId;
                newAlarm["time"] = alarmTime;
                newAlarm["fileId"] = fileId;
                newAlarm["audioUrl"] = audioUrl;  // 存储 audio_url
                newAlarm["alarmId"] = alarmId;
                
                // 保存到 Flash
                if (writeAlarmsToFlash(currentAlarms)) {
                    DEBUG_INFO(DEBUG_MODULE_ALARM, "新闹钟已添加：%s", alarmTime.c_str());
                    if (audioUrl.length() > 0) {
                        DEBUG_INFO(DEBUG_MODULE_ALARM, " (audio_url: %s)", audioUrl.c_str());
                    } else if (fileId.length() > 0) {
                        DEBUG_INFO(DEBUG_MODULE_ALARM, " (file_id: %s，无 audio_url)", fileId.c_str());
                    } else {
                        DEBUG_INFO(DEBUG_MODULE_ALARM, " (无 file_id，将播放蜂鸣声)");
                    }
                    
                    // 重置触发状态
                    m_isAlarmed = false;
                    m_lastAlarmTime = "";
                    m_lastTriggeredTaskId = "";
                } else {
                    DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：存储失败");
                    return false;
                }
            }
        }
    }
    
    if (!foundAlarmTask) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 未找到 alarm_set 类型的任务");
    }
    
    return true;
}

bool AlarmTrigger::processAlarmCancel(String taskId, String alarmId) {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] >> 处理取消闹钟任务...");
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] task_id=%s, alarm_id=%s", taskId.c_str(), alarmId.c_str());
    
    // 读取当前所有闹钟
    JsonArray currentAlarms = readAlarmsFromFlash();
    
    if (currentAlarms.size() == 0) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 当前没有闹钟，无需取消");
        return true;
    }
    
    // 查找并删除匹配的闹钟
    bool found = false;
    for (int i = 0; i < currentAlarms.size(); i++) {
        JsonObject alarm = currentAlarms[i];
        String storedTaskId = alarm["taskId"] | "";
        String storedAlarmId = alarm["alarmId"] | "";
        
        // 匹配 task_id 或 alarm_id
        if (storedTaskId == taskId || (alarmId.length() > 0 && storedAlarmId == alarmId)) {
            DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 找到要删除的闹钟：task_id=%s, time=%s", storedTaskId.c_str(), String(alarm["time"] | "").c_str());
            
            currentAlarms.remove(i);
            found = true;
            break;
        }
    }
    
    if (!found) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 未找到匹配的闹钟");
        return true;
    }
    
    // 保存更新后的闹钟列表
    if (writeAlarmsToFlash(currentAlarms)) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 闹钟已取消");
        return true;
    } else {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：保存失败");
        return false;
    }
}

void AlarmTrigger::checkAndTriggerAlarm() {
    // 读取所有闹钟
    JsonArray alarms = readAlarmsFromFlash();
    
    if (alarms.size() == 0) {
        return;
    }
    
    // 获取当前 RTC 时间
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    char currentTimeStr[6];
    sprintf(currentTimeStr, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
    String currentTime = String(currentTimeStr);
    
    // 遍历所有闹钟检查
    for (JsonObject alarm : alarms) {
        String taskId = alarm["taskId"] | "";
        String alarmTime = alarm["time"] | "";
        String fileId = alarm["fileId"] | "";
        
        if (alarmTime.length() == 0) {
            continue;
        }
        
        // 调试输出
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 检查闹钟：task_id=%s, time=%s, current=%s", taskId.c_str(), alarmTime.c_str(), currentTime.c_str());
        
        // 时间匹配检查
        if (currentTime == alarmTime) {
            // 检查是否已触发（每个闹钟独立）
            if (m_lastTriggeredTaskId != taskId) {
                DEBUG_INFO(DEBUG_MODULE_ALARM, "ALARM TRIGGERED!");
                
                // 先设置触发状态，这样回调函数才能获取到 task_id
                m_lastTriggeredTaskId = taskId;
                m_isAlarmed = true;
                m_lastAlarmTime = currentTime;
                
                // 调用回调函数
                if (m_alarmCallback != nullptr) {
                    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 调用闹钟回调...");
                    m_alarmCallback();
                } else {
                    DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：回调函数未设置！");
                }
            } else {
                DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 本分钟已触发，跳过：%s", taskId.c_str());
            }
        }
    }
    
    // 分钟跳转，重置触发状态
    if (m_isAlarmed && currentTime != m_lastAlarmTime) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 分钟跳转，重置触发状态：%s -> %s", m_lastAlarmTime.c_str(), currentTime.c_str());
        m_isAlarmed = false;
        m_lastAlarmTime = "";
        m_lastTriggeredTaskId = "";
    }
}

JsonArray AlarmTrigger::getAllAlarms() {
    return readAlarmsFromFlash();
}

bool AlarmTrigger::removeAlarm(String taskId) {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 删除单个闹钟：%s", taskId.c_str());
    
    JsonArray currentAlarms = readAlarmsFromFlash();
    
    for (int i = 0; i < currentAlarms.size(); i++) {
        if (String(currentAlarms[i]["taskId"] | "") == taskId) {
            currentAlarms.remove(i);
            return writeAlarmsToFlash(currentAlarms);
        }
    }
    
    return false;
}

void AlarmTrigger::clearAllAlarms() {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 清除所有闹钟...");
    
    Preferences preferences;
    bool opened = preferences.begin(ALARM_STORAGE_NAMESPACE, false);
    
    if (opened) {
        preferences.remove(ALARM_STORAGE_KEY_ALARMS);
        preferences.end();
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 所有闹钟已清除");
    } else {
        DEBUG_ERROR(DEBUG_MODULE_ALARM, "[AlarmTrigger] 错误：无法打开命名空间");
    }
    
    m_isAlarmed = false;
    m_lastAlarmTime = "";
    m_lastTriggeredTaskId = "";
}

String AlarmTrigger::getAudioUrlByTaskId(String taskId) {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 查询 audio_url，task_id=%s", taskId.c_str());
    
    JsonArray alarms = readAlarmsFromFlash();
    
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 闹钟数量：%d", alarms.size());
    
    for (int i = 0; i < alarms.size(); i++) {
        JsonObject alarm = alarms[i];
        String storedTaskId = alarm["taskId"] | "";
        String audioUrl = alarm["audioUrl"] | "";
        
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 检查闹钟 %d: taskId=%s, audioUrl=%s", i, storedTaskId.c_str(), audioUrl.c_str());
        
        if (storedTaskId == taskId) {
            DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 找到匹配的 task_id，返回 audio_url=%s", audioUrl.c_str());
            return audioUrl;
        }
    }
    
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 未找到匹配的 task_id，返回空字符串");
    return "";
}

String AlarmTrigger::getCurrentTriggeredTaskId() {
    return m_lastTriggeredTaskId;
}
