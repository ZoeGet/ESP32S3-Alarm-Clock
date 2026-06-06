## Why

实现一个基于压力传感器的音频播放功能，当用户施加超过500g的压力时，触发喇叭播放1秒音频，松开后再次按压可重复触发。

## What Changes

- 新增压力传感器数据采集功能，使用ESP32-S3的ADC读取RFP602柔性薄膜压力传感器的电压信号
- 新增音频播放功能，使用I2S协议驱动MAX98357A数字音频放大器
- 实现压力阈值判断和边沿触发逻辑，确保按压时只播放一次音频
- 优化ADC采样滤波，提高压力检测的稳定性

## Capabilities

### New Capabilities
- `pressure-sensing`: 压力传感器数据采集和阈值判断功能
- `audio-playback`: 通过I2S驱动MAX98357A播放音频的功能
- `trigger-logic`: 实现压力触发音频播放的边沿触发逻辑

### Modified Capabilities

## Impact

- 硬件连接：需要连接RFP602压力传感器、电压转换模块、MAX98357A音频放大器和喇叭
- 软件依赖：需要使用ESP32的ADC和I2S库
- 引脚分配：ADC使用IO4，I2S使用IO7(DIN)、IO15(BCLK)、IO16(LRC)
- 音频数据：需要预生成1秒的音频数据存储在Flash中