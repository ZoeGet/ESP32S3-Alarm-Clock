## Why

当前RTC时钟使用编译时间作为初始值，存在编译到运行的时间差（约30-60秒），导致显示时间比实际时间慢。需要引入网络时间同步机制，通过WiFi连接NTP服务器获取准确时间，消除编译时间误差，实现精确时钟显示。

## What Changes

- **新增WiFiManager智能配网功能**：ESP32自动创建AP热点，用户通过手机网页配置WiFi密码，密码自动保存到闪存
- **新增NTP时间同步功能**：连接WiFi后，从NTP服务器（阿里云/池化服务器）获取准确网络时间
- **修改RTC初始化逻辑**：优先使用NTP时间，NTP失败时回退到编译时间
- **新增WiFi连接管理**：自动连接已保存WiFi，连接失败进入配网模式
- **新增时间同步状态显示**：OLED显示配网状态、同步状态
- **保持现有模块化结构**：在RtcManager中集成NTP同步，不影响其他模块

## Capabilities

### New Capabilities
- `wifi-manager`: WiFiManager智能配网，AP模式+Web配置界面
- `ntp-time-sync`: NTP网络时间同步，支持多NTP服务器
- `wifi-connection`: WiFi连接管理，自动重连+状态监控

### Modified Capabilities
- `rtc-clock`: 修改初始化逻辑，支持NTP时间和编译时间双来源
- `oled-display`: 新增显示配网状态、同步状态、IP地址

## Impact

- **硬件依赖**：需要ESP32S3开发板（已有），无需额外RTC模块
- **软件库依赖**：新增WiFiManager库（tzapu/WiFiManager）
- **网络依赖**：首次使用需要WiFi网络，断网后RTC可独立走时
- **用户体验**：首次使用需要配网，后续自动连接，时间永远准确
- **代码结构**：修改RtcManager类，新增WiFiManager集成
