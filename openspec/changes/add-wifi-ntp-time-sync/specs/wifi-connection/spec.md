## ADDED Requirements

### Requirement: WiFi连接状态监控
系统SHALL实时监控WiFi连接状态，并在OLED上显示。

#### Scenario: 连接中状态
- **WHEN** ESP32尝试连接WiFi
- **THEN** OLED显示"WiFi Connecting..."
- **AND** 显示尝试连接的SSID

#### Scenario: 连接成功状态
- **WHEN** WiFi连接成功
- **THEN** OLED显示"WiFi Connected"
- **AND** 显示分配的IP地址（如"IP: 192.168.1.100"）
- **AND** 保持显示2秒后进入下一步

#### Scenario: 连接断开检测
- **WHEN** 运行过程中WiFi断开
- **THEN** 系统继续运行（依赖RTC）
- **AND** 下次重启时尝试重新连接

### Requirement: WiFi连接超时
系统SHALL实现连接超时机制，避免无限等待。

#### Scenario: 连接超时处理
- **WHEN** 连接WiFi超过30秒未成功
- **THEN** 判定连接失败
- **AND** 进入AP配网模式
- **AND** 保留已保存凭据（不删除）

#### Scenario: 快速连接成功
- **WHEN** WiFi信号良好且密码正确
- **THEN** 在5秒内完成连接
- **AND** 立即进入NTP同步阶段
