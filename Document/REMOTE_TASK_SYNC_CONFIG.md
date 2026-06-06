# 远程任务同步配置说明

## 重要配置步骤

在使用远程任务同步功能之前，您必须配置 API Token。

### 1. 打开配置文件

打开 `src/RemoteTaskSync.h` 文件。

### 2. 替换 API Token

找到第 14 行的以下代码：

```cpp
#define API_TOKEN "YOUR_TOKEN_HERE"  // TODO: 替换为您的实际 Token
```

将 `YOUR_TOKEN_HERE` 替换为您从服务器获取的实际 Token。

例如：

```cpp
#define API_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."  // 您的实际 Token
```

### 3. 编译并上传

保存文件后，重新编译并上传到 ESP32-S3 设备。

## API 端点配置

当前配置的 API 端点如下（如需修改，请编辑 `RemoteTaskSync.h`）：

- **基础 URL**: `https://storytoy.freefly-ai.com`
- **任务拉取接口**: `/api/v1/deviceControl/query` (GET)
- **任务更新接口**: `/api/v1/task/update` (POST)
- **轮询间隔**: 60 秒（60000 毫秒）

**重要说明**：
- 设备不是通过 URL 访问任务本身，而是通过拉取接口获取任务列表
- 任务存储在服务器数据库中
- 设备通过设备 ID（MAC 地址）标识自己
- 完整的拉取 URL：`https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68`

## 功能说明

### 三个核心函数

1. **fetchTasks()**: 从服务器获取任务列表
   - 使用 GET 请求访问 `/api/v1/task/list?device_id=[MAC 地址]`
   - 使用 WiFiClientSecure 并调用 setInsecure() 忽略证书校验
   - 请求头包含 `Authorization: Bearer [Token]`
   - 使用 ArduinoJson 解析返回的数组
   - 查找 `task_type` 为 `"alarm_set"` 的任务
   - 提取 `task_id` 和 `payload.time`

2. **saveAlarmToFlash(String time, String taskId)**: 保存闹钟时间到 Flash
   - 使用 Preferences 库
   - 命名空间为 `"alarm_data"`
   - 存储键：`"alarm_time"` 和 `"task_id"`
   - 仅当数据变化时才执行写入操作
   - 存储成功后在串口打印确认信息

3. **confirmTaskSuccess(String taskId)**: 向服务器发送任务确认
   - 使用 POST 请求访问 `/api/v1/task/update`
   - JSON 负载：`{"task_id": "taskId", "status": "success"}`
   - 在 saveAlarmToFlash 成功后立即调用

### 定时轮询逻辑

- 在 `loop()` 函数中调用 `RemoteTaskSync::loop()`
- 使用 `millis()` 实现非阻塞计时器
- 每 60 秒执行一次 `fetchTasks()`
- WiFi 断开时自动跳过，不发起请求

## 串口日志输出

启用串口监视器（波特率 115200）可查看详细的运行日志：

```
[RemoteTaskSync] 初始化远程任务同步模块...
[RemoteTaskSync] 初始化完成
[RemoteTaskSync] 设备 ID: xx:xx:xx:xx:xx:xx
[RemoteTaskSync] === 开始任务同步 ===
[RemoteTaskSync] 距离上次同步：60 秒
[RemoteTaskSync] >> 获取任务列表...
[RemoteTaskSync] 请求 URL: https://storytoy.freefly-ai.com/api/v1/task/list?device_id=xx:xx:xx:xx:xx:xx
[RemoteTaskSync] HTTP 响应码：200
[RemoteTaskSync] 响应内容：[...]
[RemoteTaskSync] 找到 alarm_set 任务：task_id=xxx, time=HH:MM:SS
[RemoteTaskSync] >> 保存闹钟时间到 Flash...
[RemoteTaskSync] 存储成功：time=HH:MM:SS, task_id=xxx
[RemoteTaskSync] >> 发送任务确认...
[RemoteTaskSync] 确认成功：...
[RemoteTaskSync] === 任务同步结束 ===
```

## 错误处理

代码包含完善的错误处理：

- **WiFi 未连接**: 跳过任务同步，打印日志
- **HTTP 请求失败**: 打印错误码，不执行后续操作
- **JSON 解析失败**: 打印详细错误信息
- **Flash 存储失败**: 打印错误日志，不发送确认
- **确认请求失败**: 打印错误码，不影响本地数据

## 与现有功能兼容

- 与 WiFiManager 配网逻辑完全兼容
- 仅在 WiFi 连接成功后执行任务同步
- 不影响 OLED 刷新、压力检测、闹钟等其他功能
- 非阻塞设计，确保主循环流畅运行

## 注意事项

1. **首次使用**: 设备会先从 Flash 读取已存储的闹钟时间（如果有）
2. **数据持久化**: 即使设备断电，存储的闹钟时间也会保留
3. **任务去重**: 只有当新任务的时间或 ID 与本地存储不同时，才会执行写入和确认
4. **安全性**: 当前使用 `setInsecure()` 忽略证书校验，生产环境建议实现完整的证书验证
