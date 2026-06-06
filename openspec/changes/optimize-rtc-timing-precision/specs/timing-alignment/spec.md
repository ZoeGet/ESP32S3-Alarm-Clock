## ADDED Requirements

### Requirement: 时间戳跟踪变量
系统SHALL维护一个全局变量`nextUpdateTime`，用于跟踪下次屏幕刷新的绝对时间戳。

#### Scenario: 变量初始化
- **WHEN** 系统初始化完成（setup()结束时）
- **THEN** `nextUpdateTime`被设置为`millis() + 1000`
- **AND** 变量类型为`unsigned long`

#### Scenario: 变量跨循环保持
- **WHEN** loop()函数多次执行
- **THEN** `nextUpdateTime`的值在调用之间保持
- **AND** 每次循环后递增1000毫秒

### Requirement: 动态延迟计算
系统SHALL根据当前时间与`nextUpdateTime`的差值，动态计算需要延迟的时间。

#### Scenario: 正常执行延迟
- **WHEN** 代码执行完成时
- **AND** `nextUpdateTime - millis() > 0`
- **THEN** 系统延迟`nextUpdateTime - millis()`毫秒

#### Scenario: 执行超时处理
- **WHEN** 代码执行完成时
- **AND** `nextUpdateTime - millis() <= 0`
- **THEN** 系统不执行延迟，立即继续
- **AND** `nextUpdateTime`仍按规律递增1000毫秒

### Requirement: 精确周期保持
系统SHALL确保每次屏幕刷新的时间间隔为精确的1000毫秒（±1ms误差）。

#### Scenario: 长期运行精度
- **WHEN** 系统连续运行10分钟
- **THEN** 屏幕刷新次数为600次（±1次）
- **AND** 累积误差不超过±10毫秒

#### Scenario: 执行时间波动补偿
- **WHEN** 单次代码执行时间波动（如10ms到30ms）
- **THEN** 下次刷新时间仍按原节奏执行
- **AND** 不因单次执行超时而改变后续节奏
