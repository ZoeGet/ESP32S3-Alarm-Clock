## MODIFIED Requirements

### Requirement: RTC初始化支持双时间来源
系统SHALL修改RTC初始化逻辑，优先使用NTP时间，NTP失败时回退到编译时间。

#### Scenario: NTP时间优先
- **WHEN** 系统初始化RTC
- **AND** WiFi已连接
- **THEN** 首先尝试从NTP同步时间
- **AND** NTP成功则使用NTP时间
- **AND** NTP失败则使用编译时间

#### Scenario: 编译时间回退
- **WHEN** NTP同步失败（无网络或服务器不可用）
- **THEN** 使用编译时间（__DATE__和__TIME__）设置RTC
- **AND** 通过串口输出警告信息
- **AND** OLED显示"Using Compile Time"

#### Scenario: 无WiFi时使用编译时间
- **WHEN** 用户跳过配网或配网失败
- **AND** 无WiFi连接
- **THEN** 直接使用编译时间初始化RTC
- **AND** 系统正常显示时间（有误差）

## REMOVED Requirements

### Requirement: 仅使用编译时间初始化
**Reason**: 编译时间存在固定误差，无法满足精确时钟需求
**Migration**: 使用NTP优先双来源策略，确保时间准确性
