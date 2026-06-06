# 闹钟数据处理与触发功能说明

## 📋 功能概述

本模块实现了远程闹钟任务的**数据解析与存储**以及**本地比对与触发**功能，是远程任务同步系统的核心组成部分。

### 核心功能

1. **数据解析与存储模块 (AlarmTrigger::processAlarmTask)**
   - 解析服务器返回的 JSON 任务列表
   - 过滤 `task_type == "alarm_set"` 的任务
   - 提取 `payload.time` 字段（格式："HH:mm"）
   - 防重复处理：对比 `task_id`，只处理新任务
   - 使用 Preferences 库存储到 "alarm_config" 命名空间

2. **本地比对与触发模块 (AlarmTrigger::checkAndTriggerAlarm)**
   - 每秒检查一次 RTC 时间
   - 格式化当前时间为 "HH:mm" 字符串
   - 与存储的闹钟时间进行比对
   - 防重复触发：同一分钟内只触发一次
   - 触发时播放 30 秒闹钟音乐

## 🏗️ 架构设计

### 模块关系

```
RemoteTaskSync (远程任务同步)
    ↓
    └─→ fetchTasks() 获取任务
         ↓
         └─→ AlarmTrigger::processAlarmTask() 处理数据
              ↓
              └─→ Preferences 存储 ("alarm_config")

main.cpp loop()
    ↓
    └─→ AlarmTrigger::checkAndTriggerAlarm() 检查触发
         ↓
         └─→ 时间匹配？
              ↓
              └─→ 是 → onAlarmTriggered() 回调
                   ↓
                   └─→ timeAlarm.playAlarmMusic() 播放音乐
```

### 数据存储结构

**命名空间**: `alarm_config`

**存储字段**:
- `time`: 闹钟时间字符串（格式："HH:mm"）
- `task_id`: 任务唯一标识符（用于防重复）

## 📝 使用示例

### 1. 初始化（在 setup() 中）

```cpp
#include "AlarmTrigger.h"

void setup() {
    // ... 其他初始化代码 ...
    
    // 初始化闹钟触发器
    AlarmTrigger::init();
    
    // 设置闹钟触发回调
    AlarmTrigger::setAlarmCallback(onAlarmTriggered);
}

// 回调函数
void onAlarmTriggered() {
    Serial.println("[Callback] 闹钟触发回调执行中...");
    timeAlarm.playAlarmMusic();
}
```

### 2. 循环检查（在 loop() 中）

```cpp
void loop() {
    // ... 其他逻辑 ...
    
    // 闹钟触发检查（每秒检查）
    AlarmTrigger::checkAndTriggerAlarm();
    
    // ... 其他逻辑 ...
}
```

### 3. 处理远程任务

```cpp
// RemoteTaskSync 模块会自动调用
String jsonResponse = "[{\"task_id\":\"123\",\"task_type\":\"alarm_set\",\"payload\":{\"time\":\"08:00\"}}]";
AlarmTrigger::processAlarmTask(jsonResponse);
// 输出：新闹钟已设置：08:00
```

## 🔍 工作流程详解

### 数据解析与存储流程

```
1. 接收 JSON 字符串
   ↓
2. ArduinoJson 解析
   ↓
3. 遍历任务数组
   ↓
4. 过滤 alarm_set 类型
   ↓
5. 提取 task_id 和 time
   ↓
6. 对比 task_id（防重复）
   ↓
   ├─→ task_id <= stored_id → 跳过
   └─→ task_id > stored_id → 继续
        ↓
7. 打开 Preferences "alarm_config"
        ↓
8. 存储 time 和 task_id
        ↓
9. 关闭 Preferences
        ↓
10. 串口打印：新闹钟已设置：[时间]
```

### 本地比对与触发流程

```
1. 读取存储的闹钟时间
   ↓
   ├─→ 未设置 → 返回
   └─→ 已设置 → 继续
        ↓
2. 获取 RTC 时间
        ↓
3. 格式化为 "HH:mm"
        ↓
4. 比对：currentTime == savedTime ?
   ↓
   ├─→ 不匹配 → 重置触发锁 (isAlarmed = false)
   └─→ 匹配 → 检查触发锁
        ↓
        ├─→ 已触发 (isAlarmed = true) → 跳过
        └─→ 未触发 (isAlarmed = false) → 触发
             ↓
5. 打印：ALARM TRIGGERED!
             ↓
6. 调用回调函数
             ↓
7. 播放闹钟音乐（30 秒）
             ↓
8. 设置触发锁 (isAlarmed = true)
```

## ⚙️ 配置选项

### 存储配置（AlarmTrigger.h）

```cpp
#define ALARM_STORAGE_NAMESPACE "alarm_config"   // 命名空间名称
#define ALARM_STORAGE_KEY_TIME "time"            // 时间字段 key
#define ALARM_STORAGE_KEY_TASK_ID "task_id"      // 任务 ID 字段 key
```

### 回调函数设置

```cpp
// 定义回调函数
void onAlarmTriggered() {
    // 自定义触发逻辑
    timeAlarm.playAlarmMusic();
}

// 设置回调
AlarmTrigger::setAlarmCallback(onAlarmTriggered);
```

## 🛡️ 防重复机制

### 任务防重复（task_id 对比）

```cpp
String storedTaskId = getStoredTaskId();
if (taskId <= storedTaskId) {
    Serial.println("任务已处理过，跳过");
    continue;  // 跳过重复任务
}
```

**优势**:
- 避免不必要的 Flash 写入
- 防止重复触发闹钟
- 减少网络确认请求

### 触发防重复（布尔标志位）

```cpp
if (!m_isAlarmed) {
    // 首次触发
    triggerAlarm();
    m_isAlarmed = true;  // 设置触发锁
} else {
    // 本分钟已触发，跳过
    Serial.println("本分钟已触发，跳过");
}

// 分钟跳转时重置
if (currentTime != savedTime && m_isAlarmed) {
    m_isAlarmed = false;  // 重置触发锁
}
```

**优势**:
- 确保同一分钟内只触发一次
- 避免 60 秒内重复播放
- 分钟跳转自动重置

## 📊 串口日志示例

### 正常设置闹钟

```
[AlarmTrigger] >> 处理闹钟任务...
[AlarmTrigger] 找到 alarm_set 任务：task_id=abc123, time=08:00
新闹钟已设置：08:00
```

### 触发闹钟

```
ALARM TRIGGERED!
[Callback] 闹钟触发回调执行中...
Playing alarm music for 30 seconds...
Alarm music finished
```

### 防重复触发

```
[AlarmTrigger] 本分钟已触发，跳过：08:00
```

### 分钟跳转重置

```
[AlarmTrigger] 分钟跳转，重置触发锁：08:00 -> 08:01
```

### 重复任务（防重复）

```
[AlarmTrigger] >> 处理闹钟任务...
[AlarmTrigger] 任务已处理过（task_id 不更新），跳过：abc123
```

## 🔧 高级功能

### 清除闹钟

```cpp
AlarmTrigger::clearAlarm();
// 输出：
// [AlarmTrigger] 清除闹钟设置...
// [AlarmTrigger] 闹钟已清除
```

### 获取存储的闹钟时间

```cpp
String alarmTime = AlarmTrigger::getStoredAlarmTime();
if (alarmTime.length() > 0) {
    Serial.print("当前闹钟时间：");
    Serial.println(alarmTime);
} else {
    Serial.println("未设置闹钟");
}
```

### 获取存储的任务 ID

```cpp
String taskId = AlarmTrigger::getStoredTaskId();
Serial.print("当前任务 ID: ");
Serial.println(taskId);
```

## ⚠️ 注意事项

### 1. 时间格式

- 服务器返回的时间格式必须为 `"HH:mm"`（24 小时制）
- 示例：`"08:00"`, `"14:30"`, `"23:59"`
- 不支持秒或 AM/PM 格式

### 2. 触发锁重置

- 触发锁在分钟跳转时自动重置
- 设备重启后触发锁会丢失（`isAlarmed = false`）
- 如果重启时已过闹钟时间，不会触发

### 3. Flash 寿命

- Preferences 库内置磨损均衡
- 防重复机制减少不必要的写入
- 建议不要频繁修改闹钟时间

### 4. 音乐播放

- 默认播放 30 秒（调用 `playOneSecondTone()` 30 次）
- 可在 `Alarm::playAlarmMusic()` 中自定义播放逻辑
- 播放期间会阻塞主循环（使用 `delay()`）

## 🧪 测试建议

### 1. 数据解析测试

```cpp
// 模拟服务器响应
String testJson = R"([
    {"task_id":"001","task_type":"alarm_set","payload":{"time":"08:00"}},
    {"task_id":"002","task_type":"other_type","payload":{}},
    {"task_id":"003","task_type":"alarm_set","payload":{"time":"14:30"}}
])";

AlarmTrigger::processAlarmTask(testJson);
// 应输出：新闹钟已设置：08:00（只处理第一个 alarm_set）
```

### 2. 触发测试

```cpp
// 手动设置一个接近当前时间的闹钟
AlarmTrigger::processAlarmTask(R"([{"task_id":"test","task_type":"alarm_set","payload":{"time":"当前时间"}}])");

// 等待触发，观察串口日志
```

### 3. 防重复测试

```cpp
// 发送相同任务两次
AlarmTrigger::processAlarmTask(R"([{"task_id":"001","task_type":"alarm_set","payload":{"time":"08:00"}}])");
AlarmTrigger::processAlarmTask(R"([{"task_id":"001","task_type":"alarm_set","payload":{"time":"08:00"}}])");
// 第二次应输出：任务已处理过，跳过
```

## 📁 相关文件

- [`AlarmTrigger.h`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\AlarmTrigger.h) - 头文件
- [`AlarmTrigger.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\AlarmTrigger.cpp) - 实现文件
- [`Alarm.h`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\Alarm.h) - 闹钟模块头文件（添加了 `playAlarmMusic()`）
- [`Alarm.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\Alarm.cpp) - 闹钟模块实现（添加了播放逻辑）
- [`RemoteTaskSync.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\RemoteTaskSync.cpp) - 集成数据处理调用
- [`main.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\main.cpp) - 集成触发检查和回调设置

## 🎯 下一步

1. **配置 API Token**（参考 `TOKEN_SETUP_GUIDE.md`）
2. **编译并上传**到 ESP32-S3 设备
3. **测试数据解析**（通过串口发送模拟 JSON）
4. **测试触发功能**（设置接近当前时间的闹钟）
5. **验证防重复机制**（发送重复任务）

如有任何问题，请查看串口日志或联系开发者！
