## ADDED Requirements

### Requirement: 闹钟与铃声文件关联
每个闹钟必须与一个唯一的 file_id 关联，SPIFFS 中的音频文件以 `/alarm_<file_id>.mp3` 格式命名。

#### Scenario: 存储闹钟时关联 file_id
- **WHEN** 收到 alarm_set 指令，包含 file_id 和 audio_url
- **THEN** 在 Flash 中存储闹钟记录，包含 taskId、time、fileId、audioUrl 字段

#### Scenario: 根据 file_id 检索音频文件
- **WHEN** 闹钟触发，需要播放铃声
- **THEN** 根据 alarm 记录中的 fileId 构造文件路径 `/alarm_<fileId>.mp3`

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

## MODIFIED Requirements

### Requirement: 网络音频播放
修改播放逻辑，从播放固定文件改为根据 file_id 播放对应文件。

#### Scenario: 播放本地音频文件
- **WHEN** 闹钟触发，且音频文件存在
- **THEN** 从 SPIFFS 播放 `/alarm_<fileId>.mp3` 文件
- **AND** 播放完成后自动停止

#### Scenario: 播放失败降级
- **WHEN** 音频文件损坏或播放失败
- **THEN** 降级播放蜂鸣声（1 秒）
