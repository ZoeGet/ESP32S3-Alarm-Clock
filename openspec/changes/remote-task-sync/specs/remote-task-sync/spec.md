## ADDED Requirements

### Requirement: 任务获取 (fetchTasks)
系统 SHALL 每 60 秒自动从云端 API 拉取任务列表，使用 HTTPS 协议进行安全通信。

**正确的 API 接口**：
- 端点：`/api/v1/deviceControl/query`
- 方法：GET
- 参数：`device_id=[MAC 地址]`（6 组 16 进制，大写）
- 完整 URL：`https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=B8:F8:62:E8:E4:68`

#### Scenario: 成功获取任务列表
- **WHEN** WiFi 连接正常且服务器可用
- **THEN** 系统成功获取任务列表并解析 JSON 数据

#### Scenario: WiFi 断开时不发起请求
- **WHEN** WiFi 连接断开
- **THEN** 系统不发起 HTTP 请求，记录日志并等待下次轮询

#### Scenario: 设备 ID 格式正确
- **WHEN** 构建请求 URL
- **THEN** 设备 ID 为 MAC 地址，格式为 6 组 16 进制（大写），如 `B8:F8:62:E8:E4:68`

#### Scenario: 过滤 alarm_set 类型任务
- **WHEN** 任务列表中包含多个任务
- **THEN** 系统仅处理 task_type 为 "alarm_set" 的任务，忽略其他类型

#### Scenario: 解析失败处理
- **WHEN** 服务器返回非 JSON 格式数据或 JSON 格式错误
- **THEN** 系统记录错误日志，不执行后续操作

### Requirement: 数据持久化 (saveAlarmToFlash)
系统 SHALL 将闹钟时间持久化存储到 Flash 中，实现掉电记忆功能。

#### Scenario: 首次存储闹钟时间
- **WHEN** 解析到新的 alarm_set 任务且 Flash 中无存储数据
- **THEN** 系统将时间字符串写入 Flash 并记录成功日志

#### Scenario: 数据变化时更新存储
- **WHEN** 新解析的时间与 Flash 中存储的时间不同
- **THEN** 系统更新 Flash 中的数据并记录变更日志

#### Scenario: 数据相同时不重复写入
- **WHEN** 新解析的时间与 Flash 中存储的时间相同
- **THEN** 系统不执行写入操作，记录跳过日志

#### Scenario: 存储失败处理
- **WHEN** Flash 写入失败
- **THEN** 系统记录错误日志，不发送任务确认

### Requirement: 任务确认回传 (confirmTaskSuccess)
系统 SHALL 在本地存储成功后立即向服务器发送任务确认，防止任务重复下发。

#### Scenario: 成功发送确认
- **WHEN** 本地存储成功且 WiFi 连接正常
- **THEN** 系统向 /api/v1/task/update 发送 POST 请求，包含 task_id 和 status: "success"

#### Scenario: 确认请求失败
- **WHEN** 服务器不可用或网络错误
- **THEN** 系统记录错误日志，不影响本地存储的数据

#### Scenario: WiFi 断开时不发送确认
- **WHEN** WiFi 连接断开
- **THEN** 系统不发起确认请求，记录跳过日志

### Requirement: 定时轮询机制
系统 SHALL 使用非阻塞定时器每 60 秒执行一次任务同步流程。

#### Scenario: 定时触发任务同步
- **WHEN** 距离上次同步时间超过 60 秒
- **THEN** 系统执行 fetchTasks() 函数

#### Scenario: 非阻塞执行
- **WHEN** 任务同步正在进行
- **THEN** 主循环的其他功能（OLED 刷新、压力检测等）不受影响

#### Scenario: 同步间隔保持
- **WHEN** 一次任务同步完成后
- **THEN** 系统重置计时器，从当前时间开始计算下一个 60 秒间隔

### Requirement: 安全配置
系统 SHALL 使用安全的通信配置与云端 API 交互。

#### Scenario: SSL 证书处理
- **WHEN** 建立 HTTPS 连接
- **THEN** 系统使用 setInsecure() 绕过证书校验以简化部署

#### Scenario: Authorization 头设置
- **WHEN** 发起 HTTP 请求
- **THEN** 系统在请求头中包含 Authorization: Bearer [Token]

#### Scenario: 设备身份标识
- **WHEN** 请求任务列表
- **THEN** 系统在 URL 中包含 device_id 参数（使用设备 MAC 地址）

### Requirement: 日志输出
系统 SHALL 提供详细的串口日志输出，便于调试和监控。

#### Scenario: 记录关键操作
- **WHEN** 执行任务获取、存储、确认操作
- **THEN** 系统通过 Serial 打印操作开始和结果

#### Scenario: 记录错误信息
- **WHEN** 发生任何错误（网络、解析、存储等）
- **THEN** 系统打印详细的错误信息和错误码

#### Scenario: 记录状态信息
- **WHEN** WiFi 状态变化或定时轮询触发
- **THEN** 系统打印当前状态和计时器信息
