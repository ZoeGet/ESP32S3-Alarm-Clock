# 闹钟播放逻辑修复

## ✅ 问题已解决

根据您的需求，已修改闹钟触发逻辑：**当 file_id 为空或不存在时，播放 1 秒蜂鸣声；否则播放闹钟音乐**。

---

## 🔧 修改内容

### 1. AlarmTrigger.h

**添加了 file_id 存储常量**：
```cpp
#define ALARM_STORAGE_KEY_FILE_ID "file_id"
```

**添加了获取 file_id 的方法**：
```cpp
static String getStoredFileId();
```

### 2. AlarmTrigger.cpp

#### 提取并存储 file_id

```cpp
// 提取 file_id（可能为空）
String fileId = command["params"]["file_id"] | "";

// 存储到 Flash
preferences.putString(ALARM_STORAGE_KEY_FILE_ID, fileId);
```

#### 串口输出优化

```cpp
if (fileId.length() > 0) {
    Serial.print(" (file_id: ");
    Serial.print(fileId);
    Serial.println(")");
} else {
    Serial.println(" (无 file_id，将播放蜂鸣声)");
}
```

#### 实现 getStoredFileId()

```cpp
String AlarmTrigger::getStoredFileId() {
    Preferences preferences;
    bool opened = preferences.begin(ALARM_STORAGE_NAMESPACE, false);
    
    if (!opened) {
        return "";
    }
    
    String fileId = preferences.getString(ALARM_STORAGE_KEY_FILE_ID, "");
    preferences.end();
    
    return fileId;
}
```

### 3. main.cpp

**修改了闹钟触发回调函数**：

```cpp
void onAlarmTriggered() {
    // 检查存储的 file_id
    String fileId = AlarmTrigger::getStoredFileId();
    
    if (fileId.length() == 0) {
        // file_id 为空或不存在，播放 1 秒蜂鸣声
        Serial.println("[Callback] file_id 为空，播放 1 秒蜂鸣声");
        speaker.playOneSecondTone();
    } else {
        // file_id 存在，播放闹钟音乐（持续 30 秒）
        Serial.print("[Callback] file_id: ");
        Serial.println(fileId);
        Serial.println("[Callback] 播放闹钟音乐（30 秒）");
        timeAlarm.playAlarmMusic();
    }
}
```

---

## 📊 工作流程

### 1. 接收任务并存储

```
服务器返回任务
  ↓
解析 JSON
  ↓
提取 time 和 file_id
  ↓
存储到 Flash
  ├─ time: "15:16"
  ├─ task_id: "xxx"
  └─ file_id: "" (可能为空)
  ↓
串口输出：新闹钟已设置：15:16 (无 file_id，将播放蜂鸣声)
```

### 2. 闹钟触发

```
时间到达 15:16
  ↓
触发回调函数
  ↓
读取 file_id
  ↓
判断
  ├─ file_id 为空 → 播放 1 秒蜂鸣声
  └─ file_id 存在 → 播放 30 秒闹钟音乐
```

---

## 🎯 两种场景

### 场景 1：file_id 为空

**服务器任务**：
```json
{
  "command_type": "alarm_set",
  "params": {
    "time": "15:16",
    "alarm_id": "8"
    // 没有 file_id 字段
  }
}
```

**设备行为**：
```
[AlarmTrigger] 新闹钟已设置：15:16 (无 file_id，将播放蜂鸣声)

当时间到达 15:16:
[Callback] 闹钟触发回调执行中...
[Callback] file_id 为空，播放 1 秒蜂鸣声
哔——（1 秒）
```

### 场景 2：file_id 存在

**服务器任务**：
```json
{
  "command_type": "alarm_set",
  "params": {
    "time": "15:16",
    "alarm_id": "8",
    "file_id": "0702d10ab18940c5976cd1a3eed015e3"
  }
}
```

**设备行为**：
```
[AlarmTrigger] 新闹钟已设置：15:16 (file_id: 0702d10ab18940c5976cd1a3eed015e3)

当时间到达 15:16:
[Callback] 闹钟触发回调执行中...
[Callback] file_id: 0702d10ab18940c5976cd1a3eed015e3
[Callback] 播放闹钟音乐（30 秒）
播放 30 秒闹钟音乐...
```

---

## 📝 修改后的预期输出

### 设置闹钟时

```
[RemoteTaskSync] 找到 alarm_set 任务：task_id=xxx, time=15:16
[AlarmTrigger] >> 处理闹钟任务...
[AlarmTrigger] 找到 alarm_set 任务：task_id=xxx, time=15:16
新闹钟已设置：15:16 (无 file_id，将播放蜂鸣声)
```

### 闹钟触发时（无 file_id）

```
[AlarmTrigger] 时间匹配：15:16 == 15:16
ALARM TRIGGERED!
[Callback] 闹钟触发回调执行中...
[Callback] file_id 为空，播放 1 秒蜂鸣声
哔——（1 秒蜂鸣声）
```

### 闹钟触发时（有 file_id）

```
[AlarmTrigger] 时间匹配：15:16 == 15:16
ALARM TRIGGERED!
[Callback] 闹钟触发回调执行中...
[Callback] file_id: 0702d10ab18940c5976cd1a3eed015e3
[Callback] 播放闹钟音乐（30 秒）
播放 30 秒闹钟音乐...
```

---

## ✅ 完整功能清单

现在闹钟功能完全支持两种模式：

- ✅ **任务拉取**：从服务器获取任务列表
- ✅ **JSON 解析**：正确解析 command_type, id, params
- ✅ **闹钟存储**：存储 time, task_id, file_id
- ✅ **本地触发**：基于 RTC 时间比对
- ✅ **防重复触发**：同一分钟只触发一次
- ✅ **智能播放**：
  - ✅ file_id 为空 → 播放 1 秒蜂鸣声
  - ✅ file_id 存在 → 播放 30 秒闹钟音乐
- ✅ **任务确认**：向服务器发送 ACK

---

## 📁 已修改的文件

1. ✅ [`AlarmTrigger.h`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\AlarmTrigger.h) - 添加 file_id 常量和方法声明
2. ✅ [`AlarmTrigger.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\AlarmTrigger.cpp) - 实现 file_id 存储和获取
3. ✅ [`main.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\main.cpp) - 修改回调函数，根据 file_id 决定播放模式

---

## 🚀 下一步

1. **编译并上传**修改后的代码
2. **测试两种场景**：
   - 发送不带 file_id 的任务，观察是否播放 1 秒蜂鸣声
   - 发送带 file_id 的任务，观察是否播放 30 秒音乐
3. **观察串口日志**，确认 file_id 检测和播放逻辑正确

---

## 💡 关键要点

1. **file_id 字段可能为空** - 服务器可能不返回 file_id
2. **智能判断** - 根据 file_id 是否存在决定播放模式
3. **1 秒蜂鸣声** - 使用 `speaker.playOneSecondTone()`
4. **30 秒音乐** - 使用 `timeAlarm.playAlarmMusic()`
5. **详细日志** - 串口输出帮助调试

现在闹钟播放功能已经完全按照您的需求实现！🎉
