## ADDED Requirements

### Requirement: 闹钟生命周期同步
闹钟创建和删除时，必须同步管理对应的音频文件。

#### Scenario: 创建闹钟时下载音频
- **WHEN** 收到 alarm_set 指令
- **THEN** 下载音频文件（如果不存在），然后存储闹钟记录

#### Scenario: 删除闹钟时删除音频文件
- **WHEN** 收到 alarm_cancel 指令
- **THEN** 从 Flash 删除闹钟记录，并从 SPIFFS 删除 `/alarm_<fileId>.mp3` 文件

#### Scenario: 播放不存在的文件
- **WHEN** 闹钟触发，但对应的音频文件不存在
- **THEN** 降级播放蜂鸣声（1 秒）
