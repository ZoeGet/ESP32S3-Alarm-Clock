## ADDED Requirements

### Requirement: RTC模块初始化
系统SHALL在启动时初始化ESP32S3内置RTC模块，并验证其可用性。

#### Scenario: RTC初始化成功
- **WHEN** 系统启动时
- **THEN** RTC模块成功初始化
- **AND** 系统继续执行后续代码

#### Scenario: RTC初始化失败
- **WHEN** 系统启动时
- **AND** RTC模块初始化失败
- **THEN** 系统通过串口输出错误信息
- **AND** 系统进入死循环停止执行

### Requirement: RTC时间设置
系统SHALL支持设置RTC初始时间，使用程序编译时间作为默认值。

#### Scenario: 首次运行设置时间
- **WHEN** 系统首次运行（或RTC时间未设置）
- **THEN** 系统使用编译时间（__DATE__和__TIME__宏）设置RTC时间
- **AND** 时间设置成功后RTC开始独立计时

#### Scenario: 后续启动保持时间
- **WHEN** 系统非首次运行（RTC已有时间）
- **THEN** 系统不重置RTC时间
- **AND** RTC继续从上次时间继续计时

### Requirement: RTC时间读取
系统SHALL能够从RTC模块读取当前时间（时、分、秒）。

#### Scenario: 正常读取时间
- **WHEN** 系统调用时间读取函数
- **THEN** 系统从RTC获取当前小时（0-23）
- **AND** 获取当前分钟（0-59）
- **AND** 获取当前秒（0-59）
- **AND** 返回的时间值准确反映RTC计时

### Requirement: 时间格式化
系统SHALL将读取的时间格式化为HH:MM:SS字符串格式，不足两位时补零。

#### Scenario: 格式化完整时间
- **WHEN** 时间为9时5分8秒
- **THEN** 格式化结果为"09:05:08"

#### Scenario: 格式化边界时间
- **WHEN** 时间为23时59分59秒
- **THEN** 格式化结果为"23:59:59"

#### Scenario: 格式化零时
- **WHEN** 时间为0时0分0秒
- **THEN** 格式化结果为"00:00:00"
