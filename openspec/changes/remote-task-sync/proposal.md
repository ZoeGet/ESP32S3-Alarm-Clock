## Why

当前 ESP32-S3 设备的闹钟时间需要在设备本地设置，无法通过远程服务器动态下发任务。为了实现云端对设备的远程管理和任务同步，需要添加一套远程任务同步机制，使设备能够定期从服务器拉取任务列表（如闹钟设置），并在本地持久化存储后向服务器确认完成。

**重要说明**：设备不是通过 URL 直接访问任务本身，而是通过拉取接口获取任务列表。任务存储在服务器数据库中，设备通过设备 ID（MAC 地址）标识自己。

## What Changes

- 新增远程任务同步功能，包含三个核心函数：`fetchTasks()`、`saveAlarmToFlash()`、`confirmTaskSuccess()`
- 在 `loop()` 中添加非阻塞定时器，每 60 秒自动轮询服务器任务列表
- 使用 HTTPClient 和 WiFiClientSecure 与云端 API 通信
- **正确的拉取接口**：`GET https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=[MAC 地址]`
- 使用 ArduinoJson 解析服务器返回的任务数据
- 使用 Preferences 库将闹钟时间持久化存储到 Flash
- 任务处理成功后自动向服务器发送确认回传

## Capabilities

- `remote-task-sync`: 远程任务同步能力，包括任务获取、数据持久化和任务确认回传
- `http-client`: HTTP 客户端能力，用于与云端 API 进行安全通信
- `json-parsing`: JSON 数据解析能力，用于解析服务器返回的任务列表
- `flash-storage`: Flash 存储能力，用于持久化保存闹钟数据

### Modified Capabilities

## Impact

- 修改 `main.cpp`：添加远程任务同步相关函数和 loop 中的定时轮询逻辑
- 新增依赖：需要 `HTTPClient`、`WiFiClientSecure`、`ArduinoJson`、`Preferences` 库
- 需要配置 API Token 和服务器地址
- 与现有 WiFiManager 配网逻辑兼容，仅在 WiFi 连接成功时执行任务同步
- 与现有闹钟系统（Alarm.h）协同工作，通过 Flash 存储传递闹钟时间
