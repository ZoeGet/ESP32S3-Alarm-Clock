## Context

当前系统使用`RTC_Millis`软件RTC，初始时间来自编译时间宏（`__DATE__`和`__TIME__`）。由于编译到上传运行存在时间差（通常30-60秒），导致显示时间始终比实际时间慢，且误差固定无法消除。

WiFiManager是一个流行的Arduino库，提供以下功能：
- 自动创建AP热点（Captive Portal）
- Web界面配置WiFi密码
- 自动保存凭据到ESP32闪存
- 自动连接已保存WiFi

NTP（Network Time Protocol）是标准网络时间同步协议，ESP32 Arduino框架内置`configTime()`函数支持。

## Goals / Non-Goals

**Goals:**
- 实现WiFiManager智能配网，无需硬编码WiFi密码
- 从NTP服务器获取准确时间（精度±50ms）
- OLED显示配网状态和同步进度
- 断网后RTC继续独立走时
- 保持现有模块化代码结构

**Non-Goals:**
- 不支持多WiFi自动切换
- 不支持离线自动重连NTP（需手动重启）
- 不实现Web界面自定义（使用WiFiManager默认界面）
- 不添加复杂的时间区处理（固定东八区北京时间）

## Decisions

### 1. 配网流程设计：阻塞式 vs 非阻塞式

**决策**：使用阻塞式配网（setup()中完成）

**流程**：
```
setup():
  1. 初始化OLED，显示"WiFi Connecting..."
  2. 调用WiFiManager.autoConnect("ESP32-Clock")
     - 有保存WiFi：自动连接，超时进入AP模式
     - 无保存WiFi：立即创建AP热点"ESP32-Clock"
  3. 手机连接热点，浏览器自动弹出配置页（或访问192.168.4.1）
  4. 用户选择WiFi、输入密码
  5. ESP32尝试连接，成功保存密码，失败返回AP模式
  6. 连接成功，OLED显示"WiFi Connected" + IP地址
  7. 调用NTP同步时间
  8. 同步成功，OLED显示"Time Synced"
```

**理由**：
- 实现简单，WiFiManager原生支持
- 配网是初始化阶段，阻塞用户可接受
- 配网完成后才进入主循环显示时间

**替代方案**：非阻塞式（loop()中处理）
- 缺点：实现复杂，需要状态机，代码可读性差

### 2. NTP服务器选择：单服务器 vs 多服务器

**决策**：使用阿里云NTP服务器（ntp.aliyun.com）+ 备用服务器

**配置**：
```cpp
const char* ntpServer1 = "ntp.aliyun.com";
const char* ntpServer2 = "cn.pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;  // 东八区
const int daylightOffset_sec = 0;     // 无夏令时
```

**理由**：
- 阿里云NTP在国内访问稳定
- 备用服务器提高成功率
- 东八区符合中国用户习惯

**替代方案**：使用pool.ntp.org
- 缺点：国际服务器，国内访问可能不稳定

### 3. 时间同步策略：同步一次 vs 定期同步

**决策**：启动时同步一次，后续依赖RTC独立走时

**逻辑**：
```cpp
void syncTimeFromNTP() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    struct tm timeinfo;
    int retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 10) {
        delay(500);
        retry++;
    }
    if (retry < 10) {
        // 同步成功，设置RTC
        rtc.adjust(DateTime(timeinfo.tm_year + 1900, ...));
    } else {
        // 同步失败，回退到编译时间
        rtc.adjust(compileTime);
    }
}
```

**理由**：
- 简化逻辑，无需维护定时同步
- RTC精度足够日常使用（±2ppm，每天误差约0.17秒）
- 如需重新同步，用户重启设备即可

**替代方案**：每小时自动同步
- 缺点：需要维护状态机，增加代码复杂度

### 4. 错误处理：同步失败回退

**决策**：NTP同步失败时，使用编译时间作为回退

**理由**：
- 确保设备始终能显示时间，即使无网络
- 编译时间误差可接受（总比不显示好）
- OLED显示"NTP Failed, Using Compile Time"

### 5. 显示优化：配网过程可视化

**决策**：OLED实时显示配网和同步状态

**状态显示**：
```
1. "WiFi Config..."      // 进入AP模式
2. "AP: ESP32-Clock"     // 显示热点名
3. "192.168.4.1"         // 显示配置地址
4. "WiFi Connecting..."  // 尝试连接
5. "WiFi Connected"      // 连接成功
6. "IP: xxx.xxx.xxx.xxx" // 显示IP
7. "Syncing Time..."     // NTP同步中
8. "Time Synced!"        // 同步成功
```

**理由**：
- 用户友好，知道设备当前状态
- 配网过程可视化，减少用户焦虑
- 故障排查时有状态提示

## Risks / Trade-offs

**[风险] WiFiManager库体积较大** → 缓解：Flash空间充足（当前使用9.7%，剩余90%），WiFiManager增加约100KB，仍在范围内。

**[风险] 配网界面中文显示问题** → 缓解：WiFiManager默认界面为英文，如需中文需要自定义HTML，当前使用英文界面。

**[风险] 企业WiFi（802.1X）不支持** → 缓解：WiFiManager仅支持PSK（密码）认证，企业证书认证不支持，这是已知限制。

**[风险] 首次配网需要手机操作** → 缓解：这是智能配网的固有特性，相比硬编码密码，灵活性更高，用户只需操作一次。

**[权衡] 网络依赖 vs 硬件成本**
- 当前方案：依赖网络，无额外硬件成本
- 替代方案（DS3231）：不依赖网络，需购买模块（5-10元）
- 选择：当前方案更适合有WiFi的环境

## Migration Plan

1. **添加库依赖**：在platformio.ini中添加WiFiManager
2. **修改RtcManager**：
   - 添加NTP同步方法
   - 修改begin()逻辑：先尝试NTP，失败回退编译时间
   - 添加WiFi连接回调（用于状态显示）
3. **修改DisplayManager**：
   - 新增显示WiFi状态方法
   - 新增显示NTP状态方法
4. **修改main.cpp**：
   - 初始化WiFiManager
   - 调整初始化顺序：OLED → WiFiManager配网 → RTC同步 → LED
5. **测试验证**：
   - 首次启动配网流程
   - 验证NTP时间准确性
   - 验证断网后RTC走时

## Open Questions

- 是否需要支持清除已保存WiFi（重置配网）？（可通过特定按键实现）
- 是否需要显示WiFi信号强度？（可选功能）
- 是否需要支持夏令时？（中国无夏令时，当前不需要）
