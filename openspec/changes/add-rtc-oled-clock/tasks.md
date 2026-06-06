## 1. 项目配置

- [ ] 1.1 在platformio.ini中添加RTClib库依赖
- [ ] 1.2 确认U8g2库已配置（用于OLED显示）

## 2. 清理现有代码

- [ ] 2.1 删除main.cpp中的WiFi相关代码（#include <WiFi.h>、ssid、password）
- [ ] 2.2 删除NTP时间同步相关代码（time.h、ntpServer、configTime）
- [ ] 2.3 删除闹钟功能相关代码（alarm_hour、alarm_min、BUZZER_PIN、闹钟判断逻辑）
- [ ] 2.4 删除displayLocalTime函数中的日期显示和闹钟逻辑
- [ ] 2.5 保留LED闪烁功能（LedBlinker、led.begin、led.setInterval、led.setColor、led.setBrightness、led.update）

## 3. RTC时钟功能实现

- [ ] 3.1 添加RTClib头文件（#include <RTClib.h>）
- [ ] 3.2 创建RTC对象（RTC_DS3231或ESP32内置RTC适配）
- [ ] 3.3 实现RTC初始化函数，包含错误处理（初始化失败进入死循环）
- [ ] 3.4 实现编译时间解析函数（解析__DATE__和__TIME__宏）
- [ ] 3.5 实现RTC时间设置函数（首次运行时设置编译时间）
- [ ] 3.6 实现RTC时间读取函数（获取时、分、秒）

## 4. OLED显示功能实现

- [ ] 4.1 确认OLED初始化代码（u8g2.begin()）
- [ ] 4.2 实现时间格式化函数（格式化为HH:MM:SS，补零对齐）
- [ ] 4.3 实现OLED时间显示函数（清屏、设置字体、显示时间、刷新屏幕）
- [ ] 4.4 确定显示位置（水平居中约20像素，垂直约45像素）

## 5. 主程序整合

- [ ] 5.1 在setup()中调用RTC初始化
- [ ] 5.2 在setup()中调用OLED初始化（保留现有代码）
- [ ] 5.3 在setup()中保留LED初始化
- [ ] 5.4 在loop()中实现每秒读取RTC时间
- [ ] 5.5 在loop()中实现每秒刷新OLED显示
- [ ] 5.6 在loop()中保留LED更新（led.update()）
- [ ] 5.7 添加delay(1000)控制更新频率

## 6. 测试验证

- [ ] 6.1 编译代码，检查无错误
- [ ] 6.2 验证RTC时间是否正确设置（首次运行）
- [ ] 6.3 验证OLED是否正常显示时间（HH:MM:SS格式）
- [ ] 6.4 验证时间是否正确补零（如09:05:08）
- [ ] 6.5 验证LED是否正常闪烁
- [ ] 6.6 验证每秒更新一次显示
