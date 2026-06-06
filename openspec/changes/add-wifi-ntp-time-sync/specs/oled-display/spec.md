## MODIFIED Requirements

### Requirement: 显示配网状态
系统SHALL在OLED上显示WiFi配网的各个状态。

#### Scenario: AP模式状态显示
- **WHEN** ESP32进入AP配网模式
- **THEN** OLED显示"WiFi Config..."
- **AND** 第二行显示"AP: ESP32-Clock"
- **AND** 第三行显示"192.168.4.1"

#### Scenario: 连接中状态显示
- **WHEN** ESP32尝试连接WiFi
- **THEN** OLED显示"WiFi Connecting..."
- **AND** 可显示目标SSID（如果空间足够）

#### Scenario: 连接成功状态显示
- **WHEN** WiFi连接成功
- **THEN** OLED显示"WiFi Connected"
- **AND** 显示IP地址（如"IP: 192.168.x.x"）
- **AND** 保持2秒后自动进入下一状态

### Requirement: 显示时间同步状态
系统SHALL在OLED上显示NTP时间同步状态。

#### Scenario: 同步中状态显示
- **WHEN** NTP同步进行中
- **THEN** OLED显示"Syncing Time..."
- **AND** 可显示重试次数（如"Retry: 3/10"）

#### Scenario: 同步成功状态显示
- **WHEN** NTP同步成功
- **THEN** OLED显示"Time Synced!"
- **AND** 保持1秒后进入正常时间显示

#### Scenario: 同步失败状态显示
- **WHEN** NTP同步失败
- **THEN** OLED显示"NTP Failed"
- **AND** 第二行显示"Using Compile Time"
- **AND** 保持2秒后进入正常时间显示

### Requirement: 显示网络信息（可选）
系统SHALL支持在正常显示时间时，短暂显示网络状态信息。

#### Scenario: 启动后显示IP
- **WHEN** 系统完成初始化
- **AND** 首次进入正常时间显示前
- **THEN** 显示IP地址2秒
- **AND** 然后切换到时间显示

## ADDED Requirements

### Requirement: 状态显示时长控制
系统SHALL控制各状态信息的显示时长，避免信息停留过久。

#### Scenario: 自动切换显示
- **WHEN** 状态信息显示完成
- **THEN** 自动切换到下一个状态或时间显示
- **AND** 每个状态显示1-2秒（根据重要性）
