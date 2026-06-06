# API 端点修正说明

## 🚨 重要架构变更

### 问题描述

之前的代码使用了错误的 API 端点：
```cpp
#define API_TASK_LIST_ENDPOINT "/api/v1/task/list"  // ❌ 错误
```

这导致设备尝试直接访问任务资源，但实际架构并非如此。

### 正确的架构理解

**任务存储方式**：
- ✅ 任务存储在服务器数据库中
- ❌ 任务不是通过 URL 访问的文件或资源

**设备拉取方式**：
- ✅ 设备通过拉取接口获取任务列表
- ✅ 设备通过设备 ID（MAC 地址）标识自己
- ❌ 设备不是直接访问任务本身

### 正确的 API 端点

**拉取任务列表**：
```
GET https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
```

**参数说明**：
- `device_id`: 设备的 MAC 地址
- 格式：6 组 16 进制，大写，冒号分隔
- 示例：`B8:F8:62:E8:E4:68`

---

## ✅ 已修改的内容

### 1. OpenSpec 文档更新

#### proposal.md
- ✅ 添加了重要说明：设备不是通过 URL 直接访问任务
- ✅ 更新了正确的拉取接口 URL
- ✅ 说明任务存储在服务器数据库中

#### design.md
- ✅ 添加了关键架构说明
- ✅ 说明设备通过设备 ID（MAC 地址）标识自己
- ✅ 更新了正确的拉取接口

#### specs/remote-task-sync/spec.md
- ✅ 添加了正确的 API 接口说明
- ✅ 添加了设备 ID 格式要求的场景
- ✅ 更新了端点、方法、参数说明

### 2. 代码更新

#### RemoteTaskSync.h
```cpp
// ✅ 已修改
#define API_TASK_LIST_ENDPOINT "/api/v1/deviceControl/query"  // 正确的拉取接口
```

#### RemoteTaskSync.cpp
```cpp
// 构建请求 URL（已自动使用正确的端点）
String url = String(API_BASE_URL) + String(API_TASK_LIST_ENDPOINT) + 
             "?device_id=" + m_deviceId;

// 最终 URL 示例：
// https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
```

### 3. 文档更新

#### REMOTE_TASK_SYNC_CONFIG.md
- ✅ 更新了 API 端点配置说明
- ✅ 添加了重要说明章节
- ✅ 解释了架构设计

---

## 📊 架构对比

### ❌ 错误的理解

```
设备 → GET /api/v1/task/list → 访问任务列表资源
```

这种理解认为任务是一个可以通过 URL 直接访问的资源。

### ✅ 正确的理解

```
设备 → GET /api/v1/deviceControl/query?device_id=XXX → 服务器查询数据库 → 返回任务列表
```

这种理解正确反映了：
- 任务是存储在数据库中的记录
- 设备通过查询接口获取任务
- 设备通过 device_id 标识自己

---

## 🔍 完整的请求流程

### 1. 设备初始化

```cpp
RemoteTaskSync::init(deviceMAC);
// 处理设备 ID：trim() + toUpperCase()
// 输出：B8:F8:62:E8:E4:68
```

### 2. 构建请求 URL

```cpp
String url = "https://storytoy.freefly-ai.com" + 
             "/api/v1/deviceControl/query" + 
             "?device_id=" + "B8:F8:62:E8:E4:68";

// 最终 URL:
// https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
```

### 3. 发起请求

```cpp
http.begin(m_wifiClient, url);
http.addHeader("Authorization", "Bearer " + API_TOKEN);
http.addHeader("Content-Type", "application/json");

int httpResponseCode = http.GET();
```

### 4. 服务器处理

```
服务器收到请求：
GET /api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
Authorization: Bearer eyJhbGciOiJIUzI1NiIs...

服务器操作：
1. 验证 Token
2. 根据 device_id 查询数据库
3. 返回该设备的任务列表
```

### 5. 解析响应

```json
[
  {
    "task_id": "task_001",
    "task_type": "alarm_set",
    "payload": {
      "time": "08:00"
    }
  }
]
```

---

## 🎯 为什么这个修正很重要

### 1. 架构理解

- **之前**：认为任务是静态资源，可以通过 URL 访问
- **现在**：理解任务是数据库记录，需要通过查询接口获取

### 2. API 设计

- **之前**：使用 `/task/list` 暗示任务是资源集合
- **现在**：使用 `/deviceControl/query` 正确反映了查询操作

### 3. 设备标识

- **之前**：可能忽略了 device_id 的重要性
- **现在**：明确 device_id 是设备在数据库中的标识符

### 4. 错误排查

- **之前**：404 错误无法理解
- **现在**：明白是因为端点不正确

---

## 📝 测试验证

### 使用 curl 测试

```bash
# 正确的测试命令
curl -X GET "https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

**预期响应**：
```json
{
  "code": 200,
  "data": [
    {
      "task_id": "task_001",
      "task_type": "alarm_set",
      "payload": {
        "time": "08:00"
      }
    }
  ]
}
```

### 设备串口输出

上传修改后的代码，观察输出：

```
[RemoteTaskSync] 原始设备 ID: b8:f8:62:e8:e4:68
[RemoteTaskSync] 处理后设备 ID: B8:F8:62:E8:E4:68

[RemoteTaskSync] === 开始任务同步 ===
[RemoteTaskSync] >> 获取任务列表...
[RemoteTaskSync] 请求 URL: https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
最终请求的 URL: https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68
Authorization Header: Bearer eyJhbGciOiJIUzI1NiIs...
[RemoteTaskSync] HTTP 响应码：200  ← 希望看到 200！
```

---

## 🚀 下一步

1. **编译并上传**修改后的代码
2. **打开串口监视器**（波特率 115200）
3. **等待 60 秒**，观察任务同步输出
4. **确认 HTTP 响应码**为 200
5. **验证任务解析**是否成功

如果仍然有问题，请检查：
- Token 是否有效
- 设备 ID 是否在服务器数据库中注册
- 服务器端是否正确配置了路由

---

## 📁 已修改的文件

### OpenSpec 文档
- ✅ [`openspec/changes/remote-task-sync/proposal.md`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\openspec\changes\remote-task-sync\proposal.md)
- ✅ [`openspec/changes/remote-task-sync/design.md`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\openspec\changes\remote-task-sync\design.md)
- ✅ [`openspec/changes/remote-task-sync/specs/remote-task-sync/spec.md`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\openspec\changes\remote-task-sync\specs\remote-task-sync\spec.md)

### 代码文件
- ✅ [`src/RemoteTaskSync.h`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\RemoteTaskSync.h) - 更新了 API 端点常量
- ✅ [`src/RemoteTaskSync.cpp`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\src\RemoteTaskSync.cpp) - 自动使用新端点

### 文档文件
- ✅ [`REMOTE_TASK_SYNC_CONFIG.md`](file:///c:/Users/EDY/Documents/PlatformIO\Projects\joy_esp32s3\REMOTE_TASK_SYNC_CONFIG.md) - 更新了 API 端点说明
- ✅ `API_ENDPOINT_FIX.md` - 本说明文档

---

## 💡 关键要点总结

1. **任务存储**：任务在服务器数据库中，不是 URL 资源
2. **拉取方式**：设备通过查询接口获取任务列表
3. **设备标识**：使用 MAC 地址（device_id）标识设备
4. **正确端点**：`/api/v1/deviceControl/query`
5. **请求格式**：`GET /api/v1/deviceControl/query?device_id=MAC 地址`

现在代码和文档都已经修正，应该可以正常拉取任务了！
