## Why

当前网络音频播放功能存在局限性：所有闹钟共享同一个音频文件，无法为每个闹钟单独设置铃声。用户需要为每个闹钟下载独立的音频文件，并实现文件复用和自动管理。

## What Changes

- **新增功能**：每个闹钟独立对应一个铃声文件
- **存储优化**：SPIFFS 中的音频文件以 `file_id` 命名（如 `/alarm_c7e0d9894f774e90b2934e8a847ed9a5.mp3`）
- **智能下载**：收到闹钟指令后，对比 `file_id`，如果同名文件已存在则跳过下载
- **自动清理**：删除闹钟时，同步删除对应的 MP3 文件
- **降级逻辑**：如果文件不存在或损坏，降级播放蜂鸣声

## Capabilities

- `alarm-ringtone-association`: 闹钟与铃声文件的关联管理，包括文件命名、存储和检索
- `ringtone-cache-management`: 铃声缓存管理，包括文件复用检查、下载优化和自动清理
- `alarm-lifecycle-sync`: 闹钟生命周期同步，闹钟创建/删除时同步管理音频文件

### Modified Capabilities
- `network-audio-streaming`: 修改播放逻辑，从播放固定文件改为根据 file_id 播放对应文件

## Impact

- **修改文件**：
  - `src/NetworkAudioPlayer.h/cpp`: 修改文件命名和下载逻辑
  - `src/AlarmTrigger.h/cpp`: 存储 file_id 与闹钟的关联
  - `src/RemoteTaskSync.cpp`: 处理闹钟创建/删除时的文件同步
- **SPIFFS 存储**：每个唯一 file_id 对应一个 MP3 文件
- **内存占用**：根据音频文件大小（约 400-500KB），需要预留足够的 SPIFFS 空间
- **网络流量**：避免重复下载相同 file_id 的音频文件，节省流量
