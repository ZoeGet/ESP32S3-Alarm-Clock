# ESP32-S3 智能闹钟

基于 **ESP32-S3** 的 WiFi 联网智能闹钟，配备 OLED 显示屏、I2S 音频播放、压力感应止闹和云端闹钟调度。

## 功能特性

- 🕐 **OLED 显示屏** — 0.96 寸 128×64 SSD1306，I2C 接口，显示时间、MAC 地址、ADC 值和各类状态画面
- 📡 **WiFiManager 配网** —  captive portal 模式，手机扫码即可配置 WiFi，无需硬编码凭据
- 🌐 **NTP 自动校时** — 自动从阿里云 NTP 服务器同步（东八区），失败时回退到编译时间
- 💡 **NeoPixel RGB LED** — 可自由设置颜色、亮度和闪烁间隔
- 🔊 **I2S 音频播放** — MAX98357A 功放驱动，从云端下载 MP3 铃声播放，最长 30 秒超时保护
- 🎵 **PWM 蜂鸣器** — 网络音频不可用时，回退到 600 Hz 蜂鸣提示
- ✋ **压力传感器** — 模拟量压力感应，上升沿检测 + 滑动窗口 ADC 滤波，按压止闹或触发蜂鸣
- ☁️ **云端闹钟调度** — 每 60 秒轮询远程 API，支持 `alarm_set` / `alarm_cancel` 指令，执行后回传 ACK 确认
- 💾 **断电记忆** — 闹钟列表持久化存储到 NVS/Preferences，重启不丢失
- 🐛 **运行时调试控制台** — 串口命令实时调整日志级别（关闭→调试）和按模块过滤

## 硬件清单

| 组件 | 型号 / 规格 |
|---|---|
| **主控板** | ESP32-S3-DevKitC-1 或兼容板 |
| **显示屏** | 0.96 寸 OLED SSD1306 128×64，I2C 接口 |
| **音频功放** | MAX98357A I2S 功放模块 + 喇叭 |
| **LED** | WS2812B NeoPixel（或其他单线串行 LED） |
| **传感器** | 模拟量压力传感器 / 薄膜压力电阻 |
| **蜂鸣器** | 8 Ω 无源蜂鸣器（可选，作为离线回退） |

### 引脚定义

| GPIO | 功能 |
|:---:|---|
| 48 | WS2812B NeoPixel LED |
| 8 (SDA)、9 (SCL) | SSD1306 OLED 显示屏（I2C） |
| 15 (BCLK)、16 (LRC)、7 (DIN) | MAX98357A I2S 功放 |
| 5 | PWM 蜂鸣器（回退扬声器） |
| 4 | 压力传感器（模拟 ADC，12 位，11 dB 衰减） |
| 0 | *（可选）* 长按重新配网（代码中已注释禁用） |

## 快速开始

### 前置条件

- 安装 [PlatformIO](https://platformio.io/)（VS Code 扩展或命令行）
- 一根 USB 数据线连接 ESP32-S3 开发板

### 首次上电

1. 设备会自动创建一个名为 **ESP32-Clock** 的 WiFi 热点。
2. 用手机或电脑连接该热点，配网页面会自动弹出。
3. 选择你家中的 WiFi 并输入密码。
4. 闹钟会自动通过 NTP 同步时间，随后进入正常显示循环。

## 云端 API 配置

闹钟通过远程 API 实现云端调度。在 `platformio.ini` 中配置服务器地址和鉴权 Token：

```ini
build_flags =
    -D API_TOKEN_CONFIGURED="你的JWT-Token"
    -D AUDIO_SERVER_URL="https://你的服务器地址"
```

### API 接口

| 方法 | 接口路径 | 用途 |
|---|---|---|
| GET | `/api/v1/deviceControl/query?device_id=<MAC>` | 轮询待执行的闹钟任务 |
| POST | `/api/v1/deviceControl/ack` | 确认闹钟已执行 |

服务端返回的 JSON 中包含 `alarm_set`（设置闹钟）和 `alarm_cancel`（取消闹钟）指令，每个指令携带任务 ID、闹钟时间和音频 URL。

## 调试控制台

通过串口工具（115200 波特率）连接后输入以下命令：

| 命令 | 效果 |
|---|---|
| `debug looklook` | 显示当前日志级别和已启用的模块 |
| `debug off` / `debug on` | 关闭 / 开启所有调试输出 |
| `debug level 0`–`4` | 设置详细程度（关闭 / 错误 / 警告 / 信息 / 调试） |
| `debug module audio on` | 启用指定模块的调试输出 |

可用模块：`main`（主循环）、`network`（网络）、`audio`（音频）、`alarm`（闹钟）、`sensor`（传感器）、`display`（显示）、`speaker`（扬声器）、`timer`（定时器）。

## 项目结构

```
├── platformio.ini              # PlatformIO 配置、库依赖、编译参数
├── src/
│   ├── main.cpp                # 入口文件 — setup() + loop()
│   ├── Clock.h/.cpp            # 时钟（RTC_Millis）+ NTP 校时
│   ├── OLED_Display.h/.cpp     # SSD1306 OLED 显示驱动
│   ├── LED.h/.cpp              # WS2812B NeoPixel 灯控
│   ├── Speaker.h/.cpp          # PWM 蜂鸣 + I2S 正弦波
│   ├── PressureSensor.h/.cpp   # 模拟压力 ADC + 滑动窗口滤波
│   ├── NetworkAudioPlayer.h/.cpp  # ESP32-audioI2S 网络音频播放
│   ├── AlarmTrigger.h/.cpp     # 云端闹钟匹配与触发分发
│   ├── RemoteTaskSync.h/.cpp   # HTTP 轮询与闹钟持久化
│   ├── DeviceMAC.h/.cpp        # MAC 地址读取
│   ├── Timer.h/.cpp            # 主循环节拍控制（约 30 Hz）
│   └── Alarm.h                 # 本地 RTC 闹钟
├── include/
│   ├── DebugHelper.h/.cpp      # 运行时调试系统（级别 + 模块过滤）
│   └── LedBlinker.h            # LED 闪烁工具
└── openspec/                   # 规格驱动开发文档
```

## 许可证

[MIT](LICENSE)
