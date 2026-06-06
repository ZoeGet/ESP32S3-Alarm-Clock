# 404 错误排查指南

## ✅ 已修复的问题

根据您的请求，我已经对 `RemoteTaskSync::fetchTasks()` 函数进行了以下改进：

### 1. 设备 ID 处理

**修改内容**：
- ✅ 在 `init()` 中添加 `trim()` 去除首尾空格
- ✅ 在 `init()` 中添加 `toUpperCase()` 转换为大写
- ✅ 在 `fetchTasks()` 中再次确保设备 ID 干净

**修改后的代码**：
```cpp
// init() 中
m_deviceId = String(deviceId);
m_deviceId.trim();           // 去除首尾空格
m_deviceId.toUpperCase();    // 转换为大写

// fetchTasks() 中
String deviceId = m_deviceId;
deviceId.trim();
deviceId.toUpperCase();
```

### 2. Authorization 头确保添加

**修改内容**：
- ✅ 在 `http.begin(url)` 后明确添加 Authorization 头
- ✅ 使用中间变量 `authHeader` 确保正确构造

**修改后的代码**：
```cpp
http.begin(m_wifiClient, url);

// 添加 Authorization 头（关键！）
String authHeader = "Bearer " + String(API_TOKEN);
http.addHeader("Authorization", authHeader);
http.addHeader("Content-Type", "application/json");
```

### 3. 增强调试输出

**新增输出**：
```cpp
// 打印最终请求 URL 用于调试
Serial.print("最终请求的 URL: ");
Serial.println(url);
Serial.print("Authorization Header: Bearer ");
Serial.println(String(API_TOKEN).substring(0, 20) + "...");
```

---

## 🔍 404 错误可能原因

### 1. 设备 ID 格式问题

**检查点**：
- 设备 ID 是否包含空格、换行符等不可见字符
- 设备 ID 的大小写是否与服务器要求一致

**修复**：已添加 `trim()` 和 `toUpperCase()` 处理

### 2. API 端点错误

**检查 URL 是否正确**：
```
https://storytoy.freefly-ai.com/api/v1/task/list?device_id=B8:F8:62:E8:E4:68
```

**可能的端点变化**：
- `/api/v1/task/list` → `/api/v1/tasks/list`
- `/api/v1/task/list` → `/api/v2/task/list`

### 3. Authorization 头缺失或格式错误

**检查点**：
- 是否正确添加了 `Authorization` 头
- Token 格式是否正确（`Bearer [token]`）
- Token 是否过期或无效

**修复**：已确保添加 Authorization 头

### 4. 服务器配置问题

**可能原因**：
- 服务器端没有为该设备 ID 配置路由
- 服务器要求其他认证方式
- API 版本不匹配

---

## 📊 新的串口输出示例

上传修改后的代码，您将看到：

```
[RemoteTaskSync] 初始化远程任务同步模块...
[RemoteTaskSync] 原始设备 ID: b8:f8:62:e8:e4:68
[RemoteTaskSync] 处理后设备 ID: B8:F8:62:E8:E4:68
[RemoteTaskSync] 初始化完成

[RemoteTaskSync] === 开始任务同步 ===
[RemoteTaskSync] 距离上次同步：60 秒
[RemoteTaskSync] >> 获取任务列表...
[RemoteTaskSync] 请求 URL: https://storytoy.freefly-ai.com/api/v1/task/list?device_id=B8:F8:62:E8:E4:68
最终请求的 URL: https://storytoy.freefly-ai.com/api/v1/task/list?device_id=B8:F8:62:E8:E4:68
Authorization Header: Bearer eyJhbGciOiJIUzI1NiIs...
[RemoteTaskSync] HTTP 响应码：200  ← 希望看到 200！
```

---

## 🛠️ 排查步骤

### 步骤 1：检查设备 ID

观察串口输出：
```
[RemoteTaskSync] 原始设备 ID: xxx
[RemoteTaskSync] 处理后设备 ID: YYY
```

**确认**：
- 原始 ID 是否有空格或特殊字符
- 处理后的 ID 是否全大写
- ID 格式是否为 `XX:XX:XX:XX:XX:XX`

### 步骤 2：检查最终 URL

观察输出：
```
最终请求的 URL: https://storytoy.freefly-ai.com/api/v1/task/list?device_id=XXX
```

**确认**：
- URL 是否完整
- device_id 参数是否正确
- 没有多余的字符或空格

### 步骤 3：检查 Authorization 头

观察输出：
```
Authorization Header: Bearer eyJhbGciOiJIUzI1NiIs...
```

**确认**：
- Token 前 20 个字符是否正确
- 是否有 `Bearer ` 前缀

### 步骤 4：手动测试 API

使用 Postman 或 curl 测试：

```bash
curl -X GET "https://storytoy.freefly-ai.com/api/v1/task/list?device_id=B8:F8:62:E8:E4:68" \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

**如果返回 200**：说明 API 正常，问题在代码配置  
**如果返回 404**：说明 API 端点或设备 ID 有问题

---

## 🎯 常见解决方案

### 方案 1：确认 API 端点

联系服务器管理员确认正确的 API 端点：
- `/api/v1/task/list` ✅
- `/api/v1/tasks/list` ❓
- `/api/v1/device/task/list` ❓

### 方案 2：检查设备 ID 格式

有些服务器要求不同的设备 ID 格式：
- `B8:F8:62:E8:E4:68` (带冒号)
- `B8F862E8E468` (不带冒号)
- `b8:f8:62:e8:e4:68` (小写)

如果需要不带冒号的格式，修改代码：

```cpp
// 去除冒号
String deviceIdNoColon = m_deviceId;
deviceIdNoColon.replace(":", "");
deviceIdNoColon.toUpperCase();
```

### 方案 3：验证 Token 有效性

检查 Token 是否过期或无效：
1. 登录开发者平台
2. 重新生成 Token
3. 更新 `platformio.ini` 中的 `API_TOKEN_CONFIGURED`
4. 重新编译上传

### 方案 4：检查服务器日志

如果有权限，查看服务器端日志：
- 请求是否到达服务器
- 服务器返回 404 的具体原因
- 是否有路由匹配问题

---

## 📝 修改后的完整代码

### RemoteTaskSync.cpp (关键部分)

```cpp
void RemoteTaskSync::init(const char* deviceId) {
    // 处理设备 ID：去除首尾空格并转换为大写
    m_deviceId = String(deviceId);
    m_deviceId.trim();           // 去除首尾空格
    m_deviceId.toUpperCase();    // 转换为大写
    
    Serial.print("[RemoteTaskSync] 原始设备 ID: ");
    Serial.println(String(deviceId));
    Serial.print("[RemoteTaskSync] 处理后设备 ID: ");
    Serial.println(m_deviceId);
}

bool RemoteTaskSync::fetchTasks() {
    // 确保设备 ID 是干净的（再次处理）
    String deviceId = m_deviceId;
    deviceId.trim();
    deviceId.toUpperCase();
    
    // 构建请求 URL
    String url = String(API_BASE_URL) + String(API_TASK_LIST_ENDPOINT) + 
                 "?device_id=" + deviceId;
    
    HTTPClient http;
    http.begin(m_wifiClient, url);
    
    // 添加 Authorization 头（关键！）
    String authHeader = "Bearer " + String(API_TOKEN);
    http.addHeader("Authorization", authHeader);
    http.addHeader("Content-Type", "application/json");
    
    // 打印最终请求 URL 用于调试
    Serial.print("最终请求的 URL: ");
    Serial.println(url);
    
    int httpResponseCode = http.GET();
    // ...
}
```

---

## 🚀 下一步

1. **编译并上传修改后的代码**
2. **打开串口监视器**（波特率 115200）
3. **等待 60 秒**，观察任务同步输出
4. **记录以下信息**：
   - 原始设备 ID
   - 处理后设备 ID
   - 最终请求的 URL
   - Authorization Header 前缀
   - HTTP 响应码

如果仍然是 404，请提供：
- 完整的串口输出
- 手动 curl 测试的结果
- 服务器管理员确认的 API 端点

这样可以进一步定位问题！
