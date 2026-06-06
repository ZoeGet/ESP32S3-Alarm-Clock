## ADDED Requirements

### Requirement: 压力传感器数据采集
系统 SHALL 使用ESP32-S3的ADC（IO4）读取RFP602柔性薄膜压力传感器的电压信号。

#### Scenario: 压力传感器数据采集
- **WHEN** 系统初始化完成
- **THEN** 系统开始持续采集压力传感器的电压信号

### Requirement: 压力阈值判断
系统 SHALL 根据采集到的ADC值判断压力是否超过500g。

#### Scenario: 压力超过500g
- **WHEN** 压力传感器受到超过500g的压力
- **THEN** 系统检测到ADC值低于预设阈值

#### Scenario: 压力低于500g
- **WHEN** 压力传感器受到低于500g的压力
- **THEN** 系统检测到ADC值高于预设阈值

### Requirement: ADC滤波
系统 SHALL 对ADC采集值进行滤波处理，提高压力检测的稳定性。

#### Scenario: ADC滤波
- **WHEN** 系统采集压力传感器信号
- **THEN** 系统对采集值进行滑动平均滤波处理