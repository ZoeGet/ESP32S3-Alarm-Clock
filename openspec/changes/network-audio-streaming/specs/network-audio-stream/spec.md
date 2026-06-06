## ADDED Requirements

### Requirement: 网络音频流下载
系统 SHALL 支持从服务器下载 MP3 音频流。

#### Scenario: 建立 HTTPS 连接
- **WHEN** 提供有效的音频文件 URL（包含 file_id）
- **THEN** 系统 SHALL 使用 WiFiClientSecure 建立 HTTPS 连接到服务器

#### Scenario: 流式读取音频数据
- **WHEN** HTTP 响应成功（状态码 200）
- **THEN** 系统 SHALL 逐字节读取响应流，不一次性加载到内存

#### Scenario: 连接超时处理
- **WHEN** 服务器在 10 秒内未响应
- **THEN** 系统 SHALL 终止连接并返回错误

#### Scenario: 服务器错误处理
- **WHEN** 服务器返回非 200 状态码（如 404、500）
- **THEN** 系统 SHALL 记录错误并返回 false

### Requirement: MP3 解码
系统 SHALL 使用 ESP32-audioI2S 库解码 MP3 数据流。

#### Scenario: 实时 MP3 解码
- **WHEN** 接收到 MP3 编码的音频数据
- **THEN** 系统 SHALL 实时解码为 PCM 数据并输出到 I2S

#### Scenario: 解码错误处理
- **WHEN** 遇到损坏的 MP3 数据帧
- **THEN** 解码器 SHALL 跳过错误帧，继续播放后续数据

#### Scenario: 不支持的格式
- **WHEN** 服务器返回非 MP3 格式（如 WAV、AAC）
- **THEN** 解码器 SHALL 尝试播放，失败时返回错误

### Requirement: 双缓冲管理
系统 SHALL 使用双缓冲机制确保连续播放。

#### Scenario: 缓冲区切换
- **WHEN** 当前播放缓冲区数据耗尽
- **THEN** 系统 SHALL 自动切换到另一个已填充的缓冲区

#### Scenario: 缓冲区欠载
- **WHEN** 网络速度慢于播放速度，两个缓冲区都为空
- **THEN** 系统 SHALL 暂停播放，等待缓冲区填充，超时后降级为蜂鸣声

### Requirement: 播放控制
系统 SHALL 提供播放控制接口。

#### Scenario: 开始播放
- **WHEN** 调用 playAudio(file_id) 函数
- **THEN** 系统 SHALL 连接服务器并开始流式播放

#### Scenario: 停止播放
- **WHEN** 调用 stopAudio() 函数
- **THEN** 系统 SHALL 立即停止播放，关闭网络连接，释放资源

#### Scenario: 播放完成检测
- **WHEN** 音频文件播放完毕
- **THEN** 系统 SHALL 自动停止并返回完成状态

#### Scenario: 播放超时
- **WHEN** 播放时间超过 30 秒
- **THEN** 系统 SHALL 自动停止播放

### Requirement: 播放状态查询
系统 SHALL 提供播放状态查询接口。

#### Scenario: 查询播放中状态
- **WHEN** 调用 isPlaying() 函数
- **THEN** 系统 SHALL 返回当前是否正在播放

#### Scenario: 查询错误状态
- **WHEN** 播放过程中发生错误
- **THEN** 系统 SHALL 提供错误码和错误描述
