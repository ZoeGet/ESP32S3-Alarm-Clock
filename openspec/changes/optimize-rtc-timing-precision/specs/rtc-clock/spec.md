## MODIFIED Requirements

### Requirement: 主循环时序控制
系统SHALL使用动态延迟机制替代固定`delay(1000)`，确保精确的1秒刷新周期。

#### Scenario: 标准刷新周期
- **WHEN** 系统进入主循环
- **THEN** 执行LED更新和显示刷新
- **AND** 计算等待时间：`waitTime = nextUpdateTime - millis()`
- **AND** 如果`waitTime > 0`，延迟`waitTime`毫秒
- **AND** 更新`nextUpdateTime += 1000`

#### Scenario: 超时后恢复节奏
- **WHEN** 某次执行超时（`waitTime <= 0`）
- **THEN** 系统不等待，立即继续
- **AND** `nextUpdateTime`仍递增1000毫秒
- **AND** 下次刷新按新的`nextUpdateTime`执行

## REMOVED Requirements

### Requirement: 固定延迟刷新
**Reason**: 固定`delay(1000)`无法补偿代码执行时间，导致累积误差
**Migration**: 使用动态延迟机制，基于`millis()`时间戳计算等待时间
