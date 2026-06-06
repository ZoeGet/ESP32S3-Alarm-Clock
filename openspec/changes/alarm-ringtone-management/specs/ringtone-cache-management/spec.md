## ADDED Requirements

### Requirement: 铃声缓存管理
系统必须管理 SPIFFS 中的音频文件缓存，避免重复下载，并在空间不足时清理旧文件。

#### Scenario: 检查文件是否存在
- **WHEN** 收到 alarm_set 指令
- **THEN** 检查 SPIFFS 中是否存在 `/alarm_<fileId>.mp3` 文件

#### Scenario: 跳过重复下载
- **WHEN** 文件已存在且文件大小大于 0
- **THEN** 跳过下载，直接使用该文件

#### Scenario: 下载新文件
- **WHEN** 文件不存在或文件大小为 0
- **THEN** 从 audio_url 下载 MP3 文件到 `/alarm_<fileId>.mp3`

#### Scenario: 清理旧文件
- **WHEN** SPIFFS 空间不足
- **THEN** 删除最近未使用的音频文件
