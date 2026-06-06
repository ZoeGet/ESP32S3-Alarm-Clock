# ⚠️ I2S 引脚配置变更通知

## 问题原因

ESP32-S3 的 GPIO7/15/16 **不是有效的 I2S 输出引脚**，导致 I2S 注册失败：
```
E (12287) I2S: register I2S object to platform failed
```

## 新的引脚配置

### 原配置（❌ 不可用）
| 信号 | 原 GPIO | 状态 |
|------|---------|------|
| BCLK | GPIO15  | ❌ 不支持 I2S |
| LRC  | GPIO16  | ❌ 不支持 I2S |
| DIN  | GPIO7   | ❌ 不支持 I2S |

### 新配置（✅ 可用）
| 信号 | 新 GPIO | 说明 |
|------|---------|------|
| BCLK | GPIO47  | ESP32-S3 标准 I2S 时钟引脚 |
| LRC  | GPIO21  | ESP32-S3 标准 I2S 字选择引脚 |
| DIN  | GPIO48  | ESP32-S3 标准 I2S 数据输出引脚 |

## 硬件连接变更

### MAX98357A 模块需要重新接线：

```
MAX98357A     ESP32-S3
---------     ----------
LRC    --->   GPIO21  (原 GPIO16)
BCLK   --->   GPIO47  (原 GPIO15)
DIN    --->   GPIO48  (原 GPIO7)
GND    --->   GND
VIN    --->   5V
SD     --->   GND (或悬空，使用 I2S 模式)
```

## 为什么选择这些引脚？

ESP32-S3 的 I2S 外设通过 GPIO 矩阵连接到任意引脚，但以下引脚是**标准 I2S 引脚**，具有最佳性能和兼容性：

- **GPIO47/48**: 通常用于 USB 差分对，但也可以配置为 I2S
- **GPIO21**: 标准 I2S 输出引脚
- 这些引脚在 ESP32-S3 开发板上通常未被占用

## 如果不方便更改硬件连接

如果无法更改接线，可以尝试以下替代引脚组合（需要测试）：

### 替代方案 1
- BCLK: GPIO33
- LRC:  GPIO18
- DIN:  GPIO17

### 替代方案 2
- BCLK: GPIO12
- LRC:  GPIO13
- DIN:  GPIO11

**注意**：替代方案可能需要修改 `NetworkAudioPlayer.h` 中的引脚定义。

## 验证步骤

1. ✅ 更改硬件接线
2. ✅ 编译并上传新代码
3. ✅ 观察串口日志，确认没有 I2S 注册错误
4. ✅ 测试音频播放是否正常

## 预期日志输出

成功配置后，串口应该显示：
```
[NetworkAudioPlayer] 配置 I2S 引脚...
[NetworkAudioPlayer] BCLK: GPIO47
[NetworkAudioPlayer] LRC: GPIO21
[NetworkAudioPlayer] DIN: GPIO48
[NetworkAudioPlayer] I2S 引脚配置成功
[NetworkAudioPlayer] 初始化完成
```

**不应该出现**：
```
E (xxxx) I2S: register I2S object to platform failed
```

## 参考资料

- [ESP32-S3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [ESP32-audioI2S Library Documentation](https://github.com/schreibfaul1/ESP32-audioI2S)
