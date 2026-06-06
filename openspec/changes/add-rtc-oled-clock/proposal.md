## Why

当前项目需要从NTP网络时间同步改为使用ESP32S3内置RTC模块实现离线实时时钟功能，并在0.96寸OLED屏幕上显示当前时间。这样可以实现无需WiFi连接的时间显示，提高设备的独立性和可用性。

## What Changes

- **新增RTC时钟功能**：使用ESP32S3内置RTC模块，替代原有的NTP网络时间同步
- **新增OLED时间显示**：在0.96寸OLED屏幕上实时显示时:分:秒格式的时间
- **新增时间格式化**：实现时间补零格式化（如09:05:08），保证显示整齐
- **新增时间初始化**：首次运行时设置RTC初始时间（使用编译时间或手动指定）
- **移除WiFi和NTP依赖**：删除WiFi连接、NTP时间同步相关代码
- **移除闹钟功能**：删除原有的闹钟提醒功能
- **保留LED闪烁功能**：保留原有的蓝色LED呼吸灯效果

## Capabilities

### New Capabilities
- `rtc-clock`: ESP32S3内置RTC时钟管理，包括初始化、时间设置、时间读取
- `oled-display`: 0.96寸OLED屏幕时间显示，包括清屏、格式化输出、刷新显示

### Modified Capabilities
- 无

## Impact

- **硬件依赖**：需要ESP32S3开发板、0.96寸I2C OLED屏幕（SSD1306）
- **软件库依赖**：RTClib库（用于RTC操作）、U8g2库（用于OLED显示）
- **引脚配置**：SDA_PIN=8, SCL_PIN=9（I2C通信），LED_PIN=48（LED控制）
- **主程序结构**：main.cpp将简化为仅包含LED闪烁和RTC时间显示功能
