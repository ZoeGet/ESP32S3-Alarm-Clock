## ADDED Requirements

### Requirement: NTP服务器配置
系统SHALL配置多个NTP服务器以提高时间同步成功率。

#### Scenario: 配置主备服务器
- **WHEN** 系统初始化NTP
- **THEN** 配置主服务器为"ntp.aliyun.com"
- **AND** 配置备用服务器为"cn.pool.ntp.org"
- **AND** 设置时区为东八区（GMT+8）
- **AND** 夏令时偏移为0

### Requirement: NTP时间同步
系统SHALL在WiFi连接成功后，从NTP服务器获取准确时间并设置RTC。

#### Scenario: 同步成功
- **WHEN** WiFi连接成功
- **AND** 调用NTP同步函数
- **THEN** 等待NTP响应（最多5秒超时）
- **AND** 获取到网络时间后设置RTC
- **AND** OLED显示"Time Synced!"

#### Scenario: 同步重试
- **WHEN** 首次NTP同步失败
- **THEN** 自动重试最多10次
- **AND** 每次重试间隔500毫秒
- **AND** 任意一次成功即停止重试

#### Scenario: 同步失败回退
- **WHEN** NTP同步10次均失败
- **THEN** 使用编译时间作为回退
- **AND** OLED显示"NTP Failed"
- **AND** 系统继续运行显示时间

### Requirement: 时间精度保证
系统SHALL确保NTP同步后的时间精度在可接受范围内。

#### Scenario: 同步精度
- **WHEN** NTP同步成功
- **THEN** 时间精度达到±50毫秒
- **AND** 与标准时间误差小于1秒

#### Scenario: 长期走时精度
- **WHEN** NTP同步后，系统独立运行24小时
- **THEN** RTC累积误差不超过±2秒
- **AND** 显示时间仍保持可用精度
