## ADDED Requirements

### Requirement: AP热点创建
系统SHALL在没有保存WiFi凭据或连接失败时，自动创建AP热点供用户配网。

#### Scenario: 首次启动创建热点
- **WHEN** 系统首次启动（无保存WiFi）
- **THEN** ESP32创建AP热点，SSID为"ESP32-Clock"
- **AND** 热点IP地址为192.168.4.1
- **AND** OLED显示"AP: ESP32-Clock"和IP地址

#### Scenario: 连接失败创建热点
- **WHEN** 已保存WiFi连接超时（30秒）
- **THEN** ESP32进入AP模式创建热点
- **AND** 保留已保存凭据不删除

### Requirement: Web配置界面
系统SHALL提供Web界面供用户选择WiFi并输入密码。

#### Scenario: 自动弹出配置页
- **WHEN** 手机连接ESP32热点
- **AND** 浏览器访问192.168.4.1或任意网址
- **THEN** 自动跳转到WiFi配置页面（Captive Portal）
- **AND** 页面显示附近可用WiFi列表

#### Scenario: 手动输入配置
- **WHEN** 用户在Web界面选择WiFi并输入密码
- **AND** 点击"Save"按钮
- **THEN** ESP32尝试连接指定WiFi
- **AND** OLED显示"WiFi Connecting..."

### Requirement: 凭据保存与自动连接
系统SHALL自动保存成功连接的WiFi凭据到闪存，并在下次启动时自动连接。

#### Scenario: 保存WiFi凭据
- **WHEN** WiFi连接成功
- **THEN** SSID和密码自动保存到ESP32闪存
- **AND** 保存后无需再次配网

#### Scenario: 自动连接已保存WiFi
- **WHEN** 系统重启（有保存WiFi）
- **THEN** 自动尝试连接已保存WiFi
- **AND** 连接成功后直接进入NTP同步
- **AND** 整个过程无需用户干预

### Requirement: 配网超时处理
系统SHALL在配网过程中实现超时机制，避免无限等待。

#### Scenario: 连接超时
- **WHEN** 用户输入密码后，ESP32尝试连接
- **AND** 连接尝试超过30秒
- **THEN** 连接失败，返回AP模式
- **AND** OLED显示"WiFi Failed, Retry..."

#### Scenario: 配网取消
- **WHEN** 用户在Web界面点击"Exit"或关闭页面
- **THEN** ESP32保持AP模式等待重新配网
- **AND** 可通过重新访问192.168.4.1继续配网
