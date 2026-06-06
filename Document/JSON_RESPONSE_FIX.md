# JSON 响应格式修正说明

## ✅ 问题已解决

根据您的串口输出，服务器返回的 JSON 格式是：

```json
{
  "code": 0,
  "message": "success",
  "data": {
    "device_id": "B8:F8:62:E8:E4:68",
    "timestamp": 1774249374.5571377,
    "commands": [
      {
        "id": "4021e70224184d14a0e40d7fcebddc71",
        "command_type": "alarm_set",
        "timestamp": 1774220545.193424,
        "params": {
          "file_id": "0702d10ab18940c5976cd1a3eed015e3",
          "alarm_id": "8",
          "time": "15:10",
          "rrule": "DAILY;INTERVAL=1",
          "volume": 50
        }
      }
    ]
  }
}
```

## 🔧 字段名称对照表

| 之前的错误字段 | 正确的服务器字段 | 说明 |
|--------------|----------------|------|
| `task_type` | `command_type` | 任务类型字段名称 |
| `task_id` | `id` | 任务唯一标识符 |
| `payload` | `params` | 任务参数对象 |
| 直接是数组 | `data.commands` | 任务数组在嵌套结构中 |

## ✅ 已修改的代码

### 1. RemoteTaskSync.cpp

**修改前**：
```cpp
// ❌ 错误的解析逻辑
if (!doc.is<JsonArray>()) {
    Serial.println("响应不是数组格式");
    return false;
}

JsonArray tasks = doc.as<JsonArray>();

for (JsonObject task : tasks) {
    taskType = task["task_type"] | "";
    taskId = task["task_id"] | "";
    alarmTime = task["payload"]["time"] | "";
}
```

**修改后**：
```cpp
// ✅ 正确的解析逻辑
// 检查响应结构：{"code": 0, "message": "success", "data": {"commands": [...]}}
if (!doc.containsKey("data") || !doc["data"].containsKey("commands")) {
    Serial.println("响应格式不正确，缺少 data.commands");
    return false;
}

// 获取 commands 数组
JsonArray commands = doc["data"]["commands"].as<JsonArray>();

for (JsonObject command : commands) {
    taskType = command["command_type"] | "";  // 使用 command_type
    taskId = command["id"] | "";              // 使用 id
    alarmTime = command["params"]["time"] | "";  // 使用 params
}
```

### 2. AlarmTrigger.cpp

**修改前**：
```cpp
// ❌ 错误的解析逻辑
if (!doc.is<JsonArray>()) {
    Serial.println("响应不是数组格式");
    return false;
}

JsonArray tasks = doc.as<JsonArray>();

for (JsonObject task : tasks) {
    taskType = task["task_type"] | "";
    taskId = task["task_id"] | "";
    alarmTime = task["payload"]["time"] | "";
}
```

**修改后**：
```cpp
// ✅ 正确的解析逻辑
// 检查响应结构：{"code": 0, "message": "success", "data": {"commands": [...]}}
if (!doc.containsKey("data") || !doc["data"].containsKey("commands")) {
    Serial.println("响应格式不正确，缺少 data.commands");
    return false;
}

// 获取 commands 数组
JsonArray commands = doc["data"]["commands"].as<JsonArray>();

for (JsonObject command : commands) {
    taskType = command["command_type"] | "";  // 使用 command_type
    taskId = command["id"] | "";              // 使用 id
    alarmTime = command["params"]["time"] | "";  // 使用 params
}
```

## 📊 完整的解析流程

### 步骤 1：接收响应

```
GET /api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
↓
服务器返回 JSON
```

### 步骤 2：解析 JSON 结构

```cpp
JsonDocument doc;
deserializeJson(doc, responsePayload);
```

### 步骤 3：验证结构

```cpp
if (!doc.containsKey("data") || !doc["data"].containsKey("commands")) {
    // 格式错误
    return false;
}
```

### 步骤 4：提取 commands 数组

```cpp
JsonArray commands = doc["data"]["commands"].as<JsonArray>();
```

### 步骤 5：遍历并过滤

```cpp
for (JsonObject command : commands) {
    String taskType = command["command_type"] | "";
    
    if (taskType == "alarm_set") {
        // 找到闹钟任务
        String taskId = command["id"] | "";
        String alarmTime = command["params"]["time"] | "";
        
        // 处理任务...
    }
}
```

## 🎯 字段映射总结

### 服务器返回格式（PowerShell 创建的任务）

```powershell
$taskBody = @{
    device_id = "B8:F8:62:E8:E4:68"
    task_type = "alarm_set"          # 创建时使用 task_type
    payload = @{
        alarm_id = "8"
        file_id = "0702d10ab18940c5976cd1a3eed015e3"
        time = "15:10"
        rrule = "DAILY;INTERVAL=1"
        volume = 50
    }
}
```

### 服务器返回格式（设备拉取到的响应）

```json
{
  "code": 0,
  "message": "success",
  "data": {
    "commands": [
      {
        "id": "xxx",                    # 注意：返回的是 id
        "command_type": "alarm_set",    # 注意：返回的是 command_type
        "params": {                     # 注意：返回的是 params
          "alarm_id": "8",
          "file_id": "xxx",
          "time": "15:10",
          "rrule": "DAILY;INTERVAL=1",
          "volume": 50
        }
      }
    ]
  }
}
```

**重要发现**：
- 创建任务时使用 `task_type`，但服务器返回时使用 `command_type`
- 创建任务时使用 `payload`，但服务器返回时使用 `params`
- 任务 ID 字段是 `id`，不是 `task_id`

## 📝 修改后的预期输出

上传修改后的代码，再次运行：

```
[RemoteTaskSync] === 开始任务同步 ===
[RemoteTaskSync] 距离上次同步：60 秒
[RemoteTaskSync] >> 获取任务列表...
[RemoteTaskSync] 请求 URL: https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
最终请求的 URL: https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
Authorization Header: Bearer eyJhbGciOiJIUzI1NiIs...
[RemoteTaskSync] HTTP 响应码：200
[RemoteTaskSync] 响应内容：{"code":0,"message":"success","data":{...}}
[RemoteTaskSync] 找到 alarm_set 任务：task_id=4021e70224184d14a0e40d7fcebddc71, time=15:10
新闹钟已设置：15:10
[RemoteTaskSync] >> 发送任务确认...
[RemoteTaskSync] 确认成功：...
[RemoteTaskSync] === 任务同步结束 ===
```

## ✅ 修改完成清单

- [x] RemoteTaskSync.cpp - 修改 JSON 解析逻辑
- [x] AlarmTrigger.cpp - 修改 JSON 解析逻辑
- [x] 字段名称映射：`task_type` → `command_type`
- [x] 字段名称映射：`task_id` → `id`
- [x] 字段名称映射：`payload` → `params`
- [x] 响应结构：直接数组 → `data.commands`

## 🚀 下一步

1. **编译并上传**修改后的代码
2. **打开串口监视器**（波特率 115200）
3. **等待 60 秒**，观察任务同步输出
4. **确认成功解析**：
   - 看到"找到 alarm_set 任务"
   - 看到"新闹钟已设置：[时间]"
   - 看到任务确认成功

现在代码已经完全修正，可以正确解析服务器返回的 JSON 格式了！
