## ADDED Requirements

### Requirement: 闹钟音频播放触发
系统 SHALL 在闹钟触发时根据 file_id 选择播放方式。

#### Scenario: file_id 存在时播放音频
- **WHEN** 闹钟触发且 file_id 不为空
- **THEN** 系统 SHALL 调用网络音频播放模块播放 MP3 音频

#### Scenario: file_id 为空时播放蜂鸣声
- **WHEN** 闹钟触发且 file_id 为空或不存在
- **THEN** 系统 SHALL 调用蜂鸣声播放函数（1 秒）

#### Scenario: 音频播放失败降级
- **WHEN** 网络音频播放失败（网络错误、服务器错误等）
- **THEN** 系统 SHALL 自动切换到蜂鸣声播放

### Requirement: 音频服务器配置
系统 SHALL 配置音频服务器的基础 URL。

#### Scenario: 构建音频文件 URL
- **WHEN** 提供 file_id
- **THEN** 系统 SHALL 拼接完整的音频文件 URL：`https://storytoy.freefly-ai.com/api/audio/{file_id}`

#### Scenario: 服务器认证
- **WHEN** 请求音频文件
- **THEN** 系统 SHALL 在 HTTP 请求头中添加 Authorization: Bearer Token

### Requirement: 播放并发控制
系统 SHALL 防止多个闹钟同时触发时的音频冲突。

#### Scenario: 播放互斥
- **WHEN** 一个闹钟正在播放音频
- **THEN** 其他闹钟触发时 SHALL 等待当前播放结束或跳过

#### Scenario: 播放中断
- **WHEN** 用户手动停止闹钟
- **THEN** 系统 SHALL 立即停止音频播放并关闭连接

### Requirement: 资源清理
系统 SHALL 在播放结束后清理资源。

#### Scenario: 播放完成清理
- **WHEN** 音频播放完成
- **THEN** 系统 SHALL 关闭网络连接、释放缓冲区、关闭 I2S

#### Scenario: 异常清理
- **WHEN** 播放过程中发生错误
- **THEN** 系统 SHALL 清理所有已分配资源，避免内存泄漏

### Requirement: 日志和调试
系统 SHALL 提供详细的播放日志。

#### Scenario: 播放开始日志
- **WHEN** 开始播放音频
- **THEN** 系统 SHALL 记录 file_id、URL、预计时长

#### Scenario: 播放进度日志
- **WHEN** 播放进行到 25%、50%、75%、100%
- **THEN** 系统 SHALL 可选记录进度日志（调试模式）

#### Scenario: 错误日志
- **WHEN** 播放过程中发生任何错误
- **THEN** 系统 SHALL 记录错误类型、错误码和上下文信息
