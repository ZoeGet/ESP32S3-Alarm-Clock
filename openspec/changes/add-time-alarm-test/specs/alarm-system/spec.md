## ADDED Requirements

### Requirement: 提醒时间设置
系统SHALL支持设置目标提醒时间，精确到秒。

#### Scenario: 时间设置
- **WHEN** 系统初始化时
- **THEN** 通过代码配置目标提醒时间（时:分:秒）
- **AND** 保存到内存变量
- **AND** OLED显示提醒时间设置状态

#### Scenario: 时间验证
- **WHEN** 设置提醒时间
- **THEN** 验证时间格式有效性（0-23时，0-59分，0-59秒）
- **AND** 无效时间自动纠正到有效范围

### Requirement: 时间到点检测
系统SHALL在主循环中每秒检测时间是否到达设定值。

#### Scenario: 时间到达检测
- **WHEN** 系统时间到达设定的提醒时间
- **THEN** 触发声音播报
- **AND** 记录触发时间
- **AND** 避免重复触发（至少间隔1分钟）

#### Scenario: 时间未到达
- **WHEN** 系统时间未到达提醒时间
- **THEN** 继续检测
- **AND** 不触发声音

### Requirement: 声音播报触发
系统SHALL在时间到达时触发喇叭发声。

#### Scenario: 正常触发
- **WHEN** 时间到达
- **THEN** 调用Speaker模块播放1kHz声音
- **AND** 持续100ms
- **AND** 发声后标记为已触发

#### Scenario: 触发间隔保护
- **WHEN** 时间到达但距离上次触发不足1分钟
- **THEN** 跳过触发
- **AND** 记录跳过原因

### Requirement: 状态显示
系统SHALL在OLED上显示提醒时间设置状态。

#### Scenario: 显示提醒时间
- **WHEN** 系统启动时
- **THEN** OLED显示"Alarm: HH:MM:SS"
- **AND** 显示当前设置的提醒时间

#### Scenario: 显示触发状态
- **WHEN** 提醒触发时
- **THEN** OLED显示"Alarm Triggered!"
- **AND** 保持显示2秒后恢复时间显示
