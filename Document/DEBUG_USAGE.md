# 调试系统使用说明

## 概述

本项目已集成了一套完整的调试系统，可以通过串口命令动态开启/关闭调试信息输出。

## 默认状态

- **默认情况下，所有调试信息都是关闭的**
- 设备运行时，串口监视器不会显示任何调试信息
- 这样可以减少串口输出对系统性能的影响

## 开启调试模式

### 方法：通过串口命令

1. 打开串口监视器（波特率 115200）
2. 输入以下命令并发送：

```
debug looklook
```

3. 当看到以下输出时，表示调试模式已成功开启：

```
[Debug] 调试模式已启用
[Debug] 已开启所有调试信息
```

## 关闭调试模式

输入以下命令：

```
debug off
```

## 其他调试命令

### 查看帮助
```
debug help
```

### 设置调试级别（0-4）
```
debug level 0    # 关闭所有调试
debug level 1    # 仅错误信息
debug level 2    # 错误 + 警告
debug level 3    # 错误 + 警告 + 信息
debug level 4    # 所有调试信息
```

### 控制特定模块的调试
```
debug module main on      # 开启主程序调试
debug module main off     # 关闭主程序调试
debug module network on   # 开启网络模块调试
debug module audio on     # 开启音频模块调试
debug module alarm on     # 开启闹钟模块调试
debug module sensor on    # 开启传感器调试
debug module speaker on   # 开启喇叭调试
debug module display on   # 开启显示模块调试
```

### 查看当前调试状态
```
debug status
```

## 调试模块分类

系统已将代码按功能模块分类：

| 模块名 | 说明 | 包含的文件 |
|--------|------|-----------|
| `main` | 主程序逻辑 | main.cpp, Clock.cpp, DeviceMAC.cpp |
| `network` | 网络连接 | RemoteTaskSync.cpp, WiFi 相关 |
| `audio` | 音频播放 | NetworkAudioPlayer.cpp |
| `alarm` | 闹钟功能 | Alarm.cpp, AlarmTrigger.cpp |
| `sensor` | 传感器 | PressureSensor.cpp |
| `speaker` | 喇叭驱动 | Speaker.cpp |
| `display` | OLED 显示 | OLED_Display.cpp |
| `timer` | 定时器 | Timer.cpp |

## 使用场景

### 场景 1：设备正常运行，不需要调试
- 保持默认状态即可
- 串口不会输出任何调试信息

### 场景 2：排查网络问题
```
# 开启所有调试
debug looklook

# 或者只开启网络模块调试
debug module network on
debug module main on
```

### 场景 3：排查音频播放问题
```
# 开启音频和喇叭调试
debug module audio on
debug module speaker on
```

### 场景 4：排查闹钟触发问题
```
# 开启闹钟模块调试
debug module alarm on
```

## 技术实现

### 文件结构
```
include/
  └── DebugHelper.h      # 调试系统头文件

src/
  └── DebugHelper.cpp    # 调试系统实现
```

### 宏定义使用

代码中使用以下宏替换了原来的 `Serial.print`：

- `DEBUG_INFO(module, format, ...)` - 信息级日志
- `DEBUG_WARNING(module, format, ...)` - 警告级日志
- `DEBUG_ERROR(module, format, ...)` - 错误级日志
- `DEBUG_PRINTF(module, format, ...)` - 格式化输出
- `DEBUG_PRINTLN(module, msg)` - 换行输出

### 示例

```cpp
// 原来的代码
Serial.println("WiFi 连接成功!");

// 修改后的代码
DEBUG_INFO(DEBUG_MODULE_NETWORK, "WiFi 连接成功!");
```

## 注意事项

1. **性能影响**：开启调试模式会略微影响系统性能，建议在调试完成后关闭
2. **串口缓冲区**：大量调试输出可能导致串口缓冲区溢出，建议及时查看
3. **电源消耗**：频繁的串口输出会增加功耗
4. **生产环境**：建议在生产环境中关闭调试模式（`debug off`）

## 故障排除

### 问题：输入命令后没有反应
- 检查串口监视器波特率是否为 115200
- 检查换行符设置（建议使用"无换行"或"NL"）
- 确保在 `loop()` 函数中调用了 `DebugHelper::handleSerialCommand()`

### 问题：调试信息输出混乱
- 关闭其他不必要的模块调试
- 降低调试级别（如 `debug level 2`）
- 及时清空串口监视器

## 扩展开发

如果需要添加新的调试模块：

1. 在 `include/DebugHelper.h` 中添加模块标志：
```cpp
#define DEBUG_MODULE_YOUR_MODULE  0x0100
```

2. 在代码中使用：
```cpp
DEBUG_INFO(DEBUG_MODULE_YOUR_MODULE, "你的调试信息");
```

3. 在 `src/DebugHelper.cpp` 的命令处理中添加模块名称映射。
