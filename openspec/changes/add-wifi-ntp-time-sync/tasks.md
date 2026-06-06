## 1. 项目配置

- [ ] 1.1 在 platformio.ini 中添加 WiFiManager 库依赖
- [ ] 1.2 确认现有库（RTClib、U8g2、Adafruit NeoPixel）已配置
- [ ] 1.3 验证 WiFi 和 time.h 头文件可用（ESP32 框架内置）

## 2. DisplayManager 扩展

- [ ] 2.1 添加 showWiFiConfig() 方法 - 显示 AP 模式信息
- [ ] 2.2 添加 showWiFiConnecting() 方法 - 显示连接中状态
- [ ] 2.3 添加 showWiFiConnected(const char* ip) 方法 - 显示连接成功和 IP
- [ ] 2.4 添加 showNTPSyncing() 方法 - 显示同步中状态
- [ ] 2.5 添加 showNTPSuccess() 方法 - 显示同步成功
- [ ] 2.6 添加 showNTPFailed() 方法 - 显示同步失败
- [ ] 2.7 添加 showCompileTimeFallback() 方法 - 显示使用编译时间

## 3. WiFiManager 集成

- [ ] 3.1 创建 WiFiManager 实例
- [ ] 3.2 配置 AP 热点名称 "ESP32-Clock"
- [ ] 3.3 实现自动连接逻辑（autoConnect）
- [ ] 3.4 添加连接超时处理（30 秒）
- [ ] 3.5 添加配网过程状态回调（显示到 OLED）

## 4. NTP 时间同步

- [ ] 4.1 配置 NTP 服务器（ntp.aliyun.com, cn.pool.ntp.org）
- [ ] 4.2 配置时区为东八区（GMT+8）
- [ ] 4.3 实现 syncTimeFromNTP() 函数
- [ ] 4.4 添加同步重试机制（最多 10 次，间隔 500ms）
- [ ] 4.5 实现 NTP 时间到 DateTime 的转换
- [ ] 4.6 添加同步失败回退到编译时间的逻辑

## 5. RtcManager 重构

- [ ] 5.1 修改 begin() 方法，支持 WiFi 和 NTP 参数
- [ ] 5.2 添加 setTimeFromNTP() 方法
- [ ] 5.3 保留 setTimeFromCompileTime() 作为回退
- [ ] 5.4 添加 DisplayManager 引用用于状态显示
- [ ] 5.5 实现双来源时间初始化逻辑（NTP 优先）

## 6. main.cpp 重构

- [ ] 6.1 调整初始化顺序：
  - 初始化串口
  - 初始化 OLED
  - 初始化 WiFiManager（配网）
  - 初始化 RTC（含 NTP 同步）
  - 初始化 LED
  - 初始化 TimingManager
- [ ] 6.2 移除硬编码 WiFi 凭据
- [ ] 6.3 添加 WiFi 连接状态检查
- [ ] 6.4 确保所有管理类正确协调工作

## 7. 编译验证

- [ ] 7.1 编译代码，检查无错误
- [ ] 7.2 检查 Flash 和 RAM 使用是否在合理范围
- [ ] 7.3 确认所有头文件引用正确

## 8. 功能测试

- [ ] 8.1 首次启动测试 - 验证 AP 模式创建
- [ ] 8.2 配网流程测试 - 手机连接热点、配置 WiFi
- [ ] 8.3 WiFi 自动连接测试 - 重启后自动连接
- [ ] 8.4 NTP 同步测试 - 验证时间准确性
- [ ] 8.5 断网运行测试 - 验证 RTC 独立走时
- [ ] 8.6 NTP 失败回退测试 - 断开网络验证编译时间回退

## 9. 优化与完善

- [ ] 9.1 优化 OLED 显示内容（避免闪烁、内容重叠）
- [ ] 9.2 添加串口调试输出（配网状态、NTP 状态）
- [ ] 9.3 调整状态显示时长（1-2 秒）
- [ ] 9.4 代码注释完善
- [ ] 9.5 更新 OpenSpec 任务完成状态
