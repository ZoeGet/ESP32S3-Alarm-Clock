## ADDED Requirements

### Requirement: 数据解析与存储 (processAlarmTask)
系统 SHALL 解析服务器返回的 JSON 字符串，提取 alarm_set 任务的时间信息并存储到 Flash 中。

#### Scenario: 成功解析 JSON 数组
- **WHEN** 输入有效的 JSON 数组字符串
- **THEN** 系统使用 ArduinoJson 成功解析并遍历数组

#### Scenario: 过滤 alarm_set 类型任务
- **WHEN** 数组中包含多个不同类型的任务
- **THEN** 系统仅处理 task_type 为 "alarm_set" 的任务，忽略其他类型

#### Scenario: 提取 payload.time 字段
- **WHEN** 找到 alarm_set 类型的任务
- **THEN** 系统从 payload 对象中提取 time 字段值（格式："HH:mm"）

#### Scenario: 防重复处理（task_id 对比）
- **WHEN** 新任务的 task_id 不大于本地存储的 task_id
- **THEN** 系统跳过该任务，不执行存储操作

#### Scenario: 存储到 Preferences
- **WHEN** 解析到新的 alarm_set 任务且通过防重复检查
- **THEN** 系统将 time 字符串以 key "time" 存储到 "alarm_config" 命名空间

#### Scenario: 存储完成后串口打印
- **WHEN** 时间字符串成功存储到 Flash
- **THEN** 系统在串口打印"新闹钟已设置：[时间]"

### Requirement: 本地比对与触发 (checkAndTriggerAlarm)
系统 SHALL 每秒检查一次当前 RTC 时间，当与存储的闹钟时间匹配时触发闹钟。

#### Scenario: 获取并格式化当前时间
- **WHEN** 执行时间检查
- **THEN** 系统从 RTC 获取当前时间并格式化为 "HH:mm" 字符串

#### Scenario: 读取存储的闹钟时间
- **WHEN** 执行比对检查
- **THEN** 系统从 "alarm_config" 命名空间读取 key 为 "time" 的存储值

#### Scenario: 时间匹配触发闹钟
- **WHEN** 当前时间字符串等于存储的时间字符串，且该分钟内尚未触发过
- **THEN** 系统在串口打印"ALARM TRIGGERED!"并调用播放函数

#### Scenario: 防重复触发（同一分钟内）
- **WHEN** 当前分钟已经触发过闹钟
- **THEN** 系统不再次触发，即使时间字符串仍然匹配

#### Scenario: 分钟跳转重置触发锁
- **WHEN** 当前时间分钟数与闹钟时间分钟数不匹配
- **THEN** 系统重置触发标志位，允许下次触发

#### Scenario: 未设置闹钟时不触发
- **WHEN** Flash 中没有存储闹钟时间
- **THEN** 系统不执行任何操作，不触发闹钟

### Requirement: 数据管理
系统 SHALL 使用 Preferences 库管理闹钟相关数据的存储和读取。

#### Scenario: 打开命名空间
- **WHEN** 需要读取或写入闹钟数据
- **THEN** 系统以读写模式打开 "alarm_config" 命名空间

#### Scenario: 存储 time 字段
- **WHEN** 设置新闹钟
- **THEN** 系统使用 putString("time", alarmTime) 存储时间

#### Scenario: 读取 time 字段
- **WHEN** 需要比对闹钟时间
- **THEN** 系统使用 getString("time", "") 读取时间

#### Scenario: 关闭命名空间
- **WHEN** 完成数据读写操作
- **THEN** 系统调用 end() 关闭命名空间，确保数据写入 Flash

### Requirement: 集成与调用
系统 SHALL 在适当的时机调用数据处理和触发函数。

#### Scenario: RemoteTaskSync 调用数据处理
- **WHEN** fetchTasks() 成功获取并解析到 alarm_set 任务
- **THEN** 系统调用 processAlarmTask() 处理数据

#### Scenario: 主循环调用触发检查
- **WHEN** 执行 loop() 中的每秒周期
- **THEN** 系统调用 checkAndTriggerAlarm() 检查是否触发闹钟

#### Scenario: 与 Alarm 模块集成
- **WHEN** 闹钟触发条件满足
- **THEN** 系统调用 Alarm 对象的播放函数
