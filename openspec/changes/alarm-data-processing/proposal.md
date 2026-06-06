## Why

当前远程任务同步功能已经实现了从服务器获取任务并存储到 Flash 的能力，但缺少两个关键环节：
1. **数据解析与存储优化**：需要更完善的 JSON 解析逻辑、防重复处理机制，以及独立的闹钟数据管理模块
2. **本地比对与触发**：需要基于 RTC 时间实现本地闹钟触发逻辑，确保闹钟在指定时间准确响起，且同一分钟内只触发一次

这两个模块是闹钟功能的核心，直接决定用户体验。

## What Changes

- 新增数据解析与存储模块：实现 `processAlarmTask()` 函数，完善 JSON 解析、防重处理和 Preferences 存储
- 新增本地比对与触发模块：实现 `checkAndTriggerAlarm()` 函数，基于 RTC 时间进行本地比对和触发
- 优化 Flash 存储结构：使用独立的命名空间 "alarm_config" 管理闹钟数据
- 实现防重复触发机制：使用布尔标志位确保同一分钟内只触发一次
- 与现有 RemoteTaskSync 模块集成：在 `loop()` 中调用闹钟触发逻辑
- 增加时间格式化功能：将 RTC 时间转换为与存储格式一致的 "HH:mm" 字符串

## Capabilities

- `alarm-data-processing`: 闹钟数据处理能力，包括 JSON 解析、防重处理、Preferences 存储
- `alarm-trigger`: 闹钟触发能力，包括时间格式化、本地比对、防重复触发、音频播放调用

### Modified Capabilities

- `remote-task-sync`: 需要与新的闹钟数据处理模块集成，RemoteTaskSync 的 saveAlarmToFlash 功能将被 processAlarmTask 替代或协同工作

## Impact

- 修改 `RemoteTaskSync.cpp`：集成新的数据处理和触发逻辑
- 修改 `main.cpp`：在 `loop()` 中添加闹钟触发检查
- 与现有 Alarm.h 模块协同工作：触发时调用 Alarm 对象的播放功能
- 与 RTC 时钟模块紧密集成：依赖 RTClib 提供准确时间
- 使用 Preferences 库进行 Flash 存储，与现有存储逻辑共享命名空间
