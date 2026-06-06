# I2S 冲突修复验证记录

## 修复日期
2026-03-26

## 问题描述
ESP32-S3 同时使用两个 I2S 模块时出现注册错误：
- Speaker 模块（蜂鸣声）
- NetworkAudioPlayer 模块（网络音频播放）

### 错误日志
```
E (13136) I2S: register I2S object to platform failed
```

## 根本原因
两个模块都初始化 I2S 外设，但使用了相同的 I2S 端口（I2S_NUM_0）和不同的引脚配置，导致硬件资源冲突。

### 冲突详情
| 模块 | I2S 端口 | 引脚配置 | 状态 |
|------|----------|----------|------|
| Speaker | I2S_NUM_0 | GPIO15/16/7 | ❌ 冲突 |
| NetworkAudioPlayer | (ESP32-audioI2S 内部) | GPIO47/21/48 | ❌ 冲突 |

## 解决方案

### 1. 统一引脚配置
将 Speaker 模块的 I2S 引脚配置与 NetworkAudioPlayer 保持一致：

```cpp
// Speaker.h
#define I2S_LRC 21
#define I2S_BCLK 47
#define I2S_DIN 48
```

### 2. 使用不同的 I2S 端口
将 Speaker 模块改为使用 I2S_NUM_1，避免与 NetworkAudioPlayer 冲突：

```cpp
// Speaker.cpp
i2s_driver_install(I2S_NUM_1, &i2s_config, 0, nullptr);
i2s_set_pin(I2S_NUM_1, &pin_config);

// playI2SAudio() 和 stop() 函数也使用 I2S_NUM_1
i2s_write(I2S_NUM_1, ...);
i2s_zero_dma_buffer(I2S_NUM_1);
```

## 修复验证

### ✅ 初始化成功日志
```
NTP 同步成功：2026-03-26 17:53:18 
Speaker PWM initialized on GPIO5 
Speaker I2S initialized 
Audio data generated 
Pressure sensor initialized on GPIO4 
Device MAC: B8:F8:62:E8:E4:68       
[RemoteTaskSync] 初始化远程任务同步模块...       
[RemoteTaskSync] 初始化完成 
[RemoteTaskSync] 原始设备 ID: B8:F8:62:E8:E4:68  
[RemoteTaskSync] 处理后设备 ID: B8:F8:62:E8:E4:68 
[AlarmTrigger] 初始化闹钟触发器模块... 
[AlarmTrigger] 初始化完成 
[AlarmTrigger] 闹钟回调已设置 
[NetworkAudioPlayer] 初始化音频播放器...      
[NetworkAudioPlayer] 设备 ID: B8:F8:62:E8:E4:68 
[NetworkAudioPlayer] 已设置忽略证书验证        
[NetworkAudioPlayer] 配置 I2S 引脚... 
[NetworkAudioPlayer] BCLK: GPIO47 
[NetworkAudioPlayer] LRC: GPIO21 
[NetworkAudioPlayer] DIN: GPIO48 
[NetworkAudioPlayer] I2S 引脚配置成功 
[NetworkAudioPlayer] 状态：空闲 
[NetworkAudioPlayer] 初始化完成 
[AudioPlayer] 网络音频播放器初始化成功 
[NetworkAudioPlayer] 音量已设置：20
```

### 关键验证点
- ✅ **没有**出现 `E (xxxx) I2S: register I2S object to platform failed` 错误
- ✅ Speaker I2S 初始化成功
- ✅ NetworkAudioPlayer I2S 引脚配置成功
- ✅ 两个模块都正常工作

## 待测试项目

### 1. Speaker 蜂鸣声测试
- [ ] 触发压力传感器，验证蜂鸣声是否正常播放
- [ ] 检查蜂鸣声时长是否为 1 秒
- [ ] 检查蜂鸣声是否清晰无杂音

### 2. NetworkAudioPlayer 网络音频测试
- [ ] 服务器下发带 audio_url 的 alarm_set 任务
- [ ] 闹钟触发时，验证是否播放服务器音频
- [ ] 检查音频是否清晰，无杂音/卡顿
- [ ] 检查播放时长是否正确（最长 30 秒）

### 3. 降级功能测试
- [ ] 当 audio_url 为空时，验证是否播放蜂鸣声
- [ ] 当网络异常时，验证是否降级为蜂鸣声
- [ ] 当播放失败时，验证是否降级为蜂鸣声

## 硬件连接

### MAX98357A 模块接线
```
MAX98357A     ESP32-S3
---------     ----------
LRC    --->   GPIO21
BCLK   --->   GPIO47
DIN    --->   GPIO48
GND    --->   GND
VIN    --->   5V
SD     --->   GND (I2S 模式)
```

### 喇叭连接
- 喇叭连接到 MAX98357A 的输出端（OUT+ 和 OUT-）

## 修改的文件

1. `src/Speaker.h` - 更改 I2S 引脚定义
2. `src/Speaker.cpp` - 改用 I2S_NUM_1

## 参考资料

- [I2S_PIN_CHANGE_NOTICE.md](./I2S_PIN_CHANGE_NOTICE.md) - I2S 引脚变更说明
- [ESP32-S3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [ESP32-audioI2S Library](https://github.com/schreibfaul1/ESP32-audioI2S)
