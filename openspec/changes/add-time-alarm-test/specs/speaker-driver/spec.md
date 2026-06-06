## ADDED Requirements

### Requirement: 喇叭驱动初始化
系统SHALL初始化PWM通道，安全驱动8Ω/0.5W喇叭。

#### Scenario: PWM初始化成功
- **WHEN** 系统启动时
- **THEN** 初始化PWM通道0，频率1kHz，8位分辨率
- **AND** 将PWM绑定到GPIO25
- **AND** 设置初始占空比为0（静音）
- **AND** 系统继续执行后续代码

#### Scenario: PWM初始化失败
- **WHEN** 系统启动时
- **AND** PWM初始化失败
- **THEN** 跳过喇叭驱动功能
- **AND** 通过串口输出错误信息
- **AND** 系统继续运行其他功能

### Requirement: 安全声音控制
系统SHALL使用低占空比PWM驱动喇叭，确保ESP32安全。

#### Scenario: 播放安全声音
- **WHEN** 调用播放声音函数
- **THEN** 使用占空比≤5%（≤13/255）
- **AND** 单次发声时间≤200ms
- **AND** 发声频率≥1Hz（最小间隔1秒）

#### Scenario: 声音参数控制
- **WHEN** 播放声音
- **THEN** 使用1kHz固定频率
- **AND** 持续时间为100ms
- **AND** 声音结束后自动停止（占空比0）

### Requirement: 错误处理
系统SHALL处理喇叭驱动的错误情况。

#### Scenario: PWM通道故障
- **WHEN** 检测到PWM通道故障
- **THEN** 跳过声音功能
- **AND** 记录错误信息
- **AND** 不影响其他功能运行

#### Scenario: 硬件连接错误
- **WHEN** 喇叭连接错误（短路/断路）
- **THEN** 系统应能正常运行
- **AND** 声音功能可能不工作，但不影响其他功能
