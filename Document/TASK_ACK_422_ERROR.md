# 任务确认接口 422 错误排查

## 🚨 问题描述

设备向确认接口发送请求时返回 422 错误：

```
[RemoteTaskSync] >> 发送任务确认...
[RemoteTaskSync] 确认 URL: https://storytoy.freefly-ai.com/api/v1/deviceControl/ack
[RemoteTaskSync] 请求体：{"task_id":"4745e3d65454491aab1a794e4d64e181","status":"success"}
[RemoteTaskSync] HTTP 响应码：422
[RemoteTaskSync] 错误：确认失败，响应码：422
```

## 📊 422 错误含义

**422 Unprocessable Entity** 表示：
- ✅ URL 正确
- ✅ HTTP 方法正确
- ✅ 认证通过
- ❌ **请求体格式不正确** 或 **缺少必需字段**

## 🔍 可能的原因

### 1. 字段名称不匹配

**当前发送**：
```json
{
  "task_id": "xxx",
  "status": "success"
}
```

**服务器可能需要**：
```json
{
  "id": "xxx",                    // 字段名是 id 不是 task_id
  "status": "completed",          // 状态值可能是 completed 不是 success
  "device_id": "B8:F8:62:E8:E4:68"  // 可能需要设备 ID
}
```

### 2. 缺少必需字段

服务器可能需要更多字段：
```json
{
  "task_id": "xxx",
  "status": "success",
  "device_id": "B8:F8:62:E8:E4:68",  // 必需
  "timestamp": 1234567890            // 必需
}
```

### 3. 状态值不正确

可能的状态值：
- `"success"` ❓
- `"completed"` ❓
- `"finished"` ❓
- `"acknowledged"` ❓
- `1` (数字) ❓

### 4. 需要额外的头信息

可能需要：
```
X-Device-Id: B8:F8:62:E8:E4:68
X-Request-Id: xxx
```

## 📝 需要向服务器管理员确认的信息

### 问题清单

1. **请求体格式**
   ```json
   {
     "task_id": "xxx",  // 字段名正确吗？
     "status": "success"  // 状态值正确吗？
   }
   ```

2. **必需的字段**
   - 是否需要 `device_id`？
   - 是否需要 `timestamp`？
   - 是否需要其他字段？

3. **状态值枚举**
   - 支持哪些状态值？
   - `"success"`, `"completed"`, `"finished"`?

4. **响应格式**
   - 成功响应是什么样的？
   - 错误响应会返回详细信息吗？

## 🔧 可以尝试的修改方案

### 方案 1：使用 `id` 而不是 `task_id`

```cpp
doc["id"] = taskId;  // 与服务器返回的字段名一致
doc["status"] = "success";
```

### 方案 2：使用 `completed` 状态

```cpp
doc["task_id"] = taskId;
doc["status"] = "completed";  // 更常见的完成状态
```

### 方案 3：添加设备 ID

```cpp
doc["task_id"] = taskId;
doc["status"] = "success";
doc["device_id"] = m_deviceId;  // 添加设备标识
```

### 方案 4：完整的请求体

```cpp
doc["id"] = taskId;
doc["status"] = "completed";
doc["device_id"] = m_deviceId;
doc["timestamp"] = time(nullptr);
```

## 🚀 建议的下一步

### 立即行动

1. **联系服务器管理员**，提供以下信息：
   - 接口 URL：`/api/v1/deviceControl/ack`
   - 当前发送的请求体：`{"task_id":"xxx","status":"success"}`
   - 返回错误：422 Unprocessable Entity
   - 请求提供正确的请求体格式

2. **查看服务器日志**（如果有权限）：
   - 服务器收到的具体请求
   - 验证失败的具体原因

3. **使用 Postman/curl 测试**：
   ```bash
   # 测试不同的请求体格式
   curl -X POST "https://storytoy.freefly-ai.com/api/v1/deviceControl/ack" \
     -H "Authorization: Bearer YOUR_TOKEN" \
     -H "Content-Type: application/json" \
     -d '{"task_id":"xxx","status":"success"}'
   ```

### 临时解决方案

如果暂时无法联系管理员，可以尝试：

```cpp
// 在 RemoteTaskSync.cpp 中修改
DynamicJsonDocument doc(512);
doc["id"] = taskId;              // 尝试使用 id
doc["status"] = "completed";     // 尝试使用 completed
doc["device_id"] = m_deviceId;   // 添加设备 ID
```

## 📋 给服务器管理员的消息模板

```
您好，

设备端任务确认接口返回 422 错误，需要确认正确的请求格式。

当前信息：
- 接口：POST /api/v1/deviceControl/ack
- 发送的请求体：{"task_id":"4745e3d65454491aab1a794e4d64e181","status":"success"}
- 返回：422 Unprocessable Entity

请提供：
1. 正确的请求体 JSON 格式
2. 必需的字段列表
3. status 字段的枚举值
4. 是否需要 device_id 或其他字段

谢谢！
```

---

## 📁 相关文件

- [`RemoteTaskSync.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\RemoteTaskSync.cpp) - 确认函数代码
- [`TASK_ACK_INTERFACE_FIX.md`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\TASK_ACK_INTERFACE_FIX.md) - 接口说明

---

## ⏸️ 当前状态

**等待服务器管理员回复**，提供正确的请求格式后才能继续修复。

**临时方案**：可以先跳过确认功能，闹钟设置和触发功能仍然正常工作。
