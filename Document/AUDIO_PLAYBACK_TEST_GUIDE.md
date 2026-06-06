# 音频播放功能测试指南

## ✅ 代码已修改完成

根据后端提供的接口文档，已修改代码以支持正确的音频播放流程。

---

## 🔄 修改内容总结

### 1. 音频播放流程变更

**之前的流程（错误）：**
```
file_id → 直接拼接 URL → 播放
```

**现在的流程（正确）：**
```
1. file_id → 请求 /api/v1/deviceControl/audio
2. 获取 302 重定向的 Location 头
3. 从 Location 获取实际音频文件地址
4. 播放音频文件
```

### 2. 修改的文件

- **NetworkAudioPlayer.h**: 添加 `getAudioPlayUrl()` 函数声明，修改 `begin()` 签名
- **NetworkAudioPlayer.cpp**: 实现 302 重定向逻辑，添加设备 ID 支持
- **main.cpp**: 传递设备 MAC 地址给音频播放器

---

## 📋 测试步骤

### 步骤 1：编译并上传代码

```bash
# 安装依赖（如果还没有安装）
pio lib install

# 编译并上传
pio run -t upload
```

### 步骤 2：打开串口监视器

- 波特率：115200
- 观察日志输出

### 步骤 3：从服务器下发音频闹钟任务

使用 PowerShell 发送任务：

```powershell
$taskBody = @{
    device_id = "B8:F8:62:E8:E4:68"
    task_type = "alarm_set"
    payload = @{
        time = "15:30"  # 设置为 1 分钟后的时间
        alarm_id = "test_audio_1"
        file_id = "your_audio_file.mp3"  # 替换为实际的 file_id
    }
} | ConvertTo-Json -Depth 10

$response = Invoke-RestMethod -Uri "https://storytoy.freefly-ai.com/api/v1/task/create" `
  -Method Post `
  -Headers @{ "Authorization" = "Bearer eyJhbGciOiJIUzI1NiIs..." } `
  -ContentType 'application/json' `
  -Body $taskBody

Write-Host "任务已下发！" -ForegroundColor Green
$response | ConvertTo-Json
```

### 步骤 4：等待设备接收任务

设备每 60 秒轮询一次任务，串口日志应该显示：

```
[RemoteTaskSync] === 开始任务同步 ===
[RemoteTaskSync] >> 获取任务列表...
[RemoteTaskSync] HTTP 响应码：200
[RemoteTaskSync] 找到 alarm_set 任务：task_id=xxx, time=15:30
[AlarmTrigger] 找到 alarm_set 任务：task_id=xxx, time=15:30, alarm_id=test_audio_1
[AlarmTrigger] 新闹钟已添加：15:30 (file_id: your_audio_file.mp3)
```

### 步骤 5：等待闹钟触发

当 RTC 时间到达 15:30 时，观察串口日志：

```
[AlarmTrigger] 检查闹钟：task_id=xxx, time=15:30, current=15:30
ALARM TRIGGERED!
[Callback] 闹钟触发回调执行中...
[Callback] file_id: your_audio_file.mp3
[Callback] 播放网络音频（最长 30 秒）
[NetworkAudioPlayer] 获取音频播放 URL，file_id=your_audio_file.mp3
[NetworkAudioPlayer] 请求 URL: https://storytoy.freefly-ai.com/api/v1/deviceControl/audio?device_id=B8:F8:62:E8:E4:68&file_id=your_audio_file.mp3
[NetworkAudioPlayer] HTTP 响应码：302
[NetworkAudioPlayer] 获取到音频 URL: https://cdn.example.com/audio/xxx.mp3
[NetworkAudioPlayer] 播放音频 URL: https://cdn.example.com/audio/xxx.mp3
[NetworkAudioPlayer] 开始连接服务器...
[NetworkAudioPlayer] 状态：播放中
[AudioInfo] Connect to new host: ...
[AudioInfo] audio_info: SampleRate: 44100
[AudioInfo] audio_info: Channels: 2
```

### 步骤 6：确认音频播放

- ✅ 如果听到喇叭播放音频：成功！
- ❌ 如果听到蜂鸣声：音频播放失败，已降级
- 🔇 如果完全没声音：检查喇叭连接和音量设置

---

## 🔍 常见问题排查

### 问题 1：获取音频 URL 失败

**日志显示：**
```
[NetworkAudioPlayer] 错误：期望 302 重定向，实际响应码：404
```

**可能原因：**
- file_id 不正确
- 音频文件不存在
- 服务器接口路径错误

**解决方法：**
1. 检查 file_id 是否正确
2. 联系后端确认接口路径
3. 使用 curl 手动测试接口

### 问题 2：Location 头为空

**日志显示：**
```
[NetworkAudioPlayer] 错误：Location 头为空
```

**可能原因：**
- 服务器返回格式不正确
- 响应头中没有 Location 字段

**解决方法：**
```bash
# 使用 curl 检查服务器响应
curl -I "https://storytoy.freefly-ai.com/api/v1/deviceControl/audio?device_id=B8:F8:62:E8:E4:68&file_id=test.mp3"
```

### 问题 3：连接音频文件失败

**日志显示：**
```
[NetworkAudioPlayer] 错误：连接失败
```

**可能原因：**
- Location URL 不可访问
- 音频文件格式不支持
- 网络连接问题

**解决方法：**
1. 检查 Location URL 是否可访问
2. 确认音频文件是 MP3 格式
3. 检查 WiFi 信号强度

### 问题 4：播放超时

**日志显示：**
```
[NetworkAudioPlayer] 播放超时，自动停止
```

**可能原因：**
- 音频文件时长超过 30 秒
- 网络速度慢导致缓冲不及时

**解决方法：**
- 使用较短的音频文件（建议 15-30 秒）
- 优化网络环境

---

## 📊 完整的日志流程示例

### 成功播放的完整日志

```
[NetworkAudioPlayer] 初始化音频播放器...
[NetworkAudioPlayer] 设备 ID: B8:F8:62:E8:E4:68
[NetworkAudioPlayer] 配置 I2S 引脚...
[NetworkAudioPlayer] BCLK: GPIO15
[NetworkAudioPlayer] LRC: GPIO16
[NetworkAudioPlayer] DIN: GPIO7
[NetworkAudioPlayer] 初始化完成

[RemoteTaskSync] === 开始任务同步 ===
[RemoteTaskSync] 找到 alarm_set 任务：task_id=abc123, time=15:30
[AlarmTrigger] 新闹钟已添加：15:30 (file_id: test.mp3)

[AlarmTrigger] 检查闹钟：task_id=abc123, time=15:30, current=15:30
ALARM TRIGGERED!
[Callback] 闹钟触发回调执行中...
[Callback] file_id: test.mp3
[Callback] 播放网络音频（最长 30 秒）
[NetworkAudioPlayer] 获取音频播放 URL，file_id=test.mp3
[NetworkAudioPlayer] 请求 URL: https://storytoy.freefly-ai.com/api/v1/deviceControl/audio?device_id=B8:F8:62:E8:E4:68&file_id=test.mp3
[NetworkAudioPlayer] HTTP 响应码：302
[NetworkAudioPlayer] 获取到音频 URL: https://cdn.example.com/audio/test.mp3
[NetworkAudioPlayer] 播放音频 URL: https://cdn.example.com/audio/test.mp3
[NetworkAudioPlayer] 开始连接服务器...
[NetworkAudioPlayer] 状态：播放中
[AudioInfo] Connect to new host: https://cdn.example.com/audio/test.mp3
[AudioInfo] audio_info: SampleRate: 44100
[AudioInfo] audio_info: Channels: 2
[AudioInfo] audio_info: BitsPerSample: 16
[NetworkAudioPlayer] 状态：播放完成

[RemoteTaskSync] >> 发送任务确认...
[RemoteTaskSync] 确认成功
```

---

## ⚠️ 重要提示

### 1. 设备 ID 格式
- 设备 ID 会自动转换为大写
- 确保与服务器端存储的 device_id 一致

### 2. 音频文件格式
- 推荐使用标准 MP3 格式
- 时长建议：15-30 秒
- 文件大小建议：< 500KB

### 3. 网络要求
- 需要稳定的 WiFi 连接
- 确保可以访问音频文件服务器（可能是 CDN）

### 4. 音量设置
- 默认音量：20（范围 0-255）
- 如需调整，修改 `setup()` 中的 `audioPlayer.setVolume(30);`

---

## 🎯 测试清单

- [ ] 编译成功，无错误
- [ ] 上传成功，设备重启
- [ ] WiFi 连接成功
- [ ] 音频播放器初始化成功
- [ ] 服务器下发音频任务
- [ ] 设备成功接收任务
- [ ] 闹钟触发
- [ ] 成功获取音频 URL（302 重定向）
- [ ] 成功连接音频文件
- [ ] 喇叭播放音频
- [ ] 播放完成后自动停止

---

## 📞 如果测试失败

请提供以下信息：

1. **完整的串口日志**
2. **服务器下发的 task_id 和 file_id**
3. **使用 curl 测试音频接口的响应**

这样可以快速定位问题！

---

**代码已准备就绪，可以开始测试了！** 🚀
