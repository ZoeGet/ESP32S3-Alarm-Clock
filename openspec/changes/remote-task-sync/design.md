## Context

当前 ESP32-S3 项目已具备 WiFi 连接能力（通过 WiFiManager）、OLED 显示、闹钟、压力传感器等功能模块。设备可以通过 SmartConfig 或 AP 配网方式连接 WiFi，并通过 NTP 同步时间。但闹钟时间需要在代码中硬编码或通过本地方式设置，无法实现云端远程下发任务。

项目已使用 Arduino 框架，依赖库包括 WiFiManager、RTClib、U8g2 等。现在需要添加远程任务同步功能，使设备能够定期从云端 API 拉取任务列表并持久化存储。

**关键架构说明**：
- 任务存储在服务器数据库中，不是通过 URL 访问具体任务文件
- 设备通过设备 ID（MAC 地址）标识自己
- 正确的拉取接口：`GET https://storytoy.freefly-ai.com/api/v1/deviceControl/query?device_id=[MAC 地址]`
- 设备通过拉取接口获取任务列表，而不是直接访问任务

## Goals / Non-Goals

**Goals:**
- 实现每 60 秒自动轮询云端 API 获取任务列表
- 解析并过滤 `task_type` 为 "alarm_set" 的任务
- 将闹钟时间持久化存储到 Flash，实现掉电记忆
- 任务处理成功后向服务器发送确认回传
- 与现有 WiFiManager 配网逻辑完全兼容
- 完善的错误处理和串口日志输出

**Non-Goals:**
- 不支持任务类型扩展（仅处理 "alarm_set" 类型）
- 不支持任务优先级或并发处理
- 不实现本地任务队列管理
- 不处理任务的取消或更新逻辑

## Decisions

### 1. HTTP 客户端选择：WiFiClientSecure + HTTPClient

**决策**: 使用 `WiFiClientSecure` 配合 `HTTPClient` 进行 HTTPS 通信。

**理由**: 
- ESP32 Arduino 框架原生支持，无需额外依赖
- `WiFiClientSecure` 提供 SSL/TLS 加密通信
- `setInsecure()` 可绕过证书校验，适合嵌入式设备简化部署

**替代方案**: 
- 使用 `HTTPClient` 非安全模式：不安全，已被排除
- 使用完整的证书验证：需要管理 CA 证书，增加复杂度

### 2. JSON 解析：ArduinoJson

**决策**: 使用 ArduinoJson 库解析服务器返回的 JSON 数据。

**理由**:
- 轻量级，适合资源受限的嵌入式设备
- 项目可能已有此依赖（ESP32 开发常用库）
- API 简洁，支持动态 JSON 文档

**替代方案**:
- 手动解析 JSON 字符串：代码复杂且易出错
- 使用其他 JSON 库：可能增加不必要的依赖

### 3. 数据存储：Preferences 库

**决策**: 使用 ESP32 原生的 `Preferences` 库进行 Flash 存储。

**理由**:
- ESP32 Arduino 框架内置，无需额外依赖
- 支持命名空间管理，数据结构清晰
- 自动处理 Flash 磨损均衡，比直接操作 NVS 更安全

**替代方案**:
- 使用 EEPROM: 需要手动管理地址和磨损均衡
- 直接操作 NVS: API 复杂，容易出错

### 4. 定时轮询：非阻塞 millis() 计时器

**决策**: 使用 `millis()` 实现非阻塞定时器，每 60 秒执行一次任务同步。

**理由**:
- 不阻塞主循环，不影响其他功能（如 OLED 刷新、压力检测）
- 符合现有代码风格（已有 Timer 类使用类似模式）
- 简单可靠，易于调试

**替代方案**:
- 使用 hardware timer: 过于复杂，需要中断处理
- 使用 delay(): 会阻塞主循环，影响用户体验

### 5. 任务去重：本地存储对比

**决策**: 只有当新任务的 time 或 task_id 与本地存储不同时，才执行写入和确认操作。

**理由**:
- 避免不必要的 Flash 写入，延长 Flash 寿命
- 避免重复向服务器发送确认，减少网络流量
- 简化逻辑，不需要维护复杂的状态机

## Risks / Trade-offs

### [风险] SSL 证书校验被绕过

**描述**: 使用 `setInsecure()` 会跳过证书验证，存在中间人攻击风险。

**缓解**: 
- 在生产环境中应实现完整的证书验证
- 当前阶段优先保证功能可用性

### [风险] Flash 写入寿命

**描述**: 频繁写入 Flash 可能导致存储单元损坏。

**缓解**: 
- 通过对比新旧值，仅在数据变化时写入
- Preferences 库内置磨损均衡机制

### [风险] 网络请求失败

**描述**: WiFi 不稳定或服务器不可用时，请求可能失败。

**缓解**: 
- 在发起请求前检查 WiFi 连接状态
- 添加超时和重试机制（当前版本先实现基础版本）
- 详细的串口日志便于调试

### [风险] Token 硬编码

**描述**: API Token 需要硬编码在代码中，存在泄露风险。

**缓解**: 
- 建议通过编译选项或配置文件管理 Token
- 未来可实现通过配网页面配置 Token

## Migration Plan

1. **添加依赖**: 在 `platformio.ini` 中添加 `ArduinoJson` 库
2. **创建头文件**: 创建 `RemoteTaskSync.h` 声明相关函数
3. **创建实现文件**: 创建 `RemoteTaskSync.cpp` 实现核心逻辑
4. **集成到 main.cpp**: 在 `loop()` 中添加定时轮询调用
5. **测试验证**: 通过串口日志验证任务获取、存储和确认流程
6. **配置 Token**: 填入实际的 API Token 和服务器地址

## Open Questions

- API Token 的具体值是什么？需要用户填入
- 是否需要支持多个闹钟任务的同时存储？
- 是否需要实现指数退避重试机制？
