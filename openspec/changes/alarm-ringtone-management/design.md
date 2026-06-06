## Context

当前网络音频播放功能已实现，但存在以下限制：
- 所有闹钟共享同一个音频文件路径（固定为 `/alarm.mp3`）
- 无法为不同闹钟设置不同的铃声
- 收到新闹钟指令时总是重新下载音频文件
- 删除闹钟时不会清理对应的音频文件

ESP32-S3 的 SPIFFS 文件系统可用空间约为 1-2MB，单个 MP3 文件大小约 400-500KB，可以存储 2-4 个音频文件。

## Goals / Non-Goals

**Goals:**
- 每个闹钟独立对应一个铃声文件，以 file_id 命名
- 智能缓存管理，避免重复下载相同 file_id 的音频
- 删除闹钟时自动清理对应的音频文件
- 保持降级逻辑：文件不存在时播放蜂鸣声

**Non-Goals:**
- 不支持用户上传自定义音频文件（file_id 由服务器生成）
- 不支持音频文件预加载（只在闹钟创建时下载）
- 不实现复杂的缓存替换算法（LRU 等）

## Decisions

### 1. 文件命名方案
**Decision**: 使用 `/alarm_<file_id>.mp3` 格式命名

**Rationale**:
- file_id 是唯一的（UUID 格式），避免文件名冲突
- 统一前缀 `/alarm_` 便于识别和管理
- 保留 `.mp3` 扩展名，ESP32-audioI2S 库需要识别文件格式

**Alternatives Considered**:
- 使用 taskId 命名：taskId 可能变化，file_id 更稳定
- 使用哈希值：增加复杂度，无实际收益

### 2. 缓存检查时机
**Decision**: 在收到 alarm_set 指令时检查文件是否存在

**Rationale**:
- 提前检查可以避免不必要的网络请求
- 下载操作在 RemoteTaskSync 阶段完成，闹钟触发时直接使用
- 减少闹钟触发时的延迟

**Alternatives Considered**:
- 闹钟触发时检查：增加触发延迟，用户体验差
- 定期后台检查：增加系统复杂度，收益不明显

### 3. 文件清理策略
**Decision**: 删除闹钟时立即删除对应音频文件

**Rationale**:
- 实现简单，逻辑清晰
- 及时释放 SPIFFS 空间
- 避免 orphan 文件积累

**Alternatives Considered**:
- LRU 缓存淘汰：实现复杂，ESP32 资源有限
- 定期清理：需要额外定时器，增加功耗
- 保留所有文件：SPIFFS 空间有限，可能存满

### 4. 错误处理
**Decision**: 文件不存在或损坏时降级播放蜂鸣声

**Rationale**:
- 保证闹钟功能始终可用
- 用户体验优于完全无声
- 与现有降级逻辑一致

## Risks / Trade-offs

**[Risk]** SPIFFS 空间不足
- **Mitigation**: 限制同时存在的闹钟数量，删除旧闹钟时释放空间

**[Risk]** 下载失败导致闹钟无铃声
- **Mitigation**: 降级播放蜂鸣声，记录错误日志

**[Risk]** file_id 重复导致文件覆盖
- **Mitigation**: file_id 由服务器生成（UUID），理论上不会重复

**[Trade-off]** 不实现预加载，首次触发可能有延迟
- **Acceptable**: 闹钟触发前已下载完成，无延迟

**[Trade-off]** 不实现复杂缓存管理，可能浪费空间
- **Acceptable**: 闹钟数量有限（通常 1-3 个），空间足够

## Migration Plan

### 部署步骤
1. 更新代码到 ESP32
2. 格式化 SPIFFS（首次启动时自动执行）
3. 旧文件 `/alarm.mp3` 自动废弃

### 回滚策略
- 回滚到旧版本代码
- 所有闹钟使用固定文件 `/alarm.mp3`
- 需要手动重新下载音频文件

## Open Questions

1. 是否需要支持多个 file_id 对应同一个音频文件？（服务器优化场景）
   - 当前方案：不支持，每个 file_id 独立文件
   - 未来扩展：可以实现文件引用计数

2. 是否需要音频文件完整性校验？
   - 当前方案：检查文件大小 > 0
   - 未来扩展：可以添加 MD5 校验
