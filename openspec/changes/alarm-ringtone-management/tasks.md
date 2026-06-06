## 1. 文件命名和存储结构

- [x] 1.1 修改 NetworkAudioPlayer.h，添加文件命名常量 `ALARM_FILE_PREFIX "/alarm_"`
- [x] 1.2 修改 downloadAudioFile 函数，使用 `/alarm_<file_id>.mp3` 格式保存文件
- [x] 1.3 删除旧的固定文件路径 `/alarm.mp3` 相关代码

## 2. 智能下载逻辑

- [x] 2.1 在 downloadAudioFile 函数中添加文件存在性检查
- [x] 2.2 如果文件已存在且大小 > 0，跳过下载，返回 true
- [x] 2.3 如果文件不存在或大小 = 0，执行下载
- [x] 2.4 添加下载进度日志，显示文件大小和下载状态

## 3. 闹钟与 file_id 关联

- [x] 3.1 修改 AlarmTrigger.h，在 AlarmData 结构中添加 fileId 字段
- [x] 3.2 修改 processAlarmTask 函数，从服务器响应中提取 file_id
- [x] 3.3 修改 saveAlarmToFlash 函数，存储 file_id 到 Flash
- [x] 3.4 添加 getFileIdByTaskId 函数，根据 task_id 检索 file_id

## 4. 播放逻辑修改

- [x] 4.1 修改 playAudioByUrl 函数，根据 file_id 构造文件路径
- [x] 4.2 使用 `connecttoFS(SPIFFS, "/alarm_<file_id>.mp3")` 播放本地文件
- [x] 4.3 添加文件不存在检测，降级播放蜂鸣声
- [x] 4.4 添加播放错误处理，文件损坏时降级

## 5. 删除闹钟时的文件清理

- [x] 5.1 修改 RemoteTaskSync.cpp，处理 alarm_cancel 命令
- [x] 5.2 从 Flash 删除闹钟记录前，先获取 file_id
- [x] 5.3 使用 SPIFFS.remove("/alarm_<file_id>.mp3") 删除音频文件
- [x] 5.4 添加删除日志，显示删除的文件名和结果

## 6. 测试和验证

- [ ] 6.1 测试场景 1：创建闹钟，验证文件下载和存储
- [ ] 6.2 测试场景 2：重复创建相同 file_id 的闹钟，验证跳过下载
- [ ] 6.3 测试场景 3：删除闹钟，验证文件同步删除
- [ ] 6.4 测试场景 4：闹钟触发，验证播放对应铃声
- [ ] 6.5 测试场景 5：文件不存在时，验证降级播放蜂鸣声
- [ ] 6.6 测试场景 6：创建多个不同 file_id 的闹钟，验证独立播放

## 7. 文档和日志

- [x] 7.1 更新代码注释，说明文件命名规则
- [ ] 7.2 添加 SPIFFS 使用说明文档
- [ ] 7.3 记录常见问题和解决方案
