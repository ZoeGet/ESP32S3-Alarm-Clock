## Why

本功能需要在设备上显示唯一的设备标识符（MAC 地址），用于设备识别和调试目的。通过从 eFuse 读取硬件 MAC 地址并在 OLED 屏幕上显示，用户可以在不连接串口的情况下快速查看设备唯一标识。

## What Changes

- 新增 MAC 地址读取功能模块，从 ESP32-S3 的 eFuse 中读取 Base MAC 地址
- 将 MAC 地址转换为有冒号、全大写的 17 位十六进制字符串格式（例如：24:0A:C4:A3:8B:FF）
- 使用静态全局变量缓存 MAC 地址，确保只在启动时读取一次硬件
- 在 OLED 屏幕上显示 MAC 地址，不与其他显示内容重叠
- 通过串口打印 MAC 地址信息

## Capabilities

- `device-mac-reader`: 从 eFuse 读取设备 MAC 地址并转换为字符串格式
- `mac-display`: 在 OLED 屏幕上显示 MAC 地址信息

### Modified Capabilities

## Impact

- 新增 `src/DeviceMAC.h` 和 `src/DeviceMAC.cpp` 文件
- 修改 `src/main.cpp` 在初始化时读取并显示 MAC 地址
- 修改 `src/OLED_Display.h` 和 `src/OLED_Display.cpp` 添加 MAC 地址显示方法
- 依赖 ESP-IDF 的 `esp_read_mac` 底层函数
- 不影响现有功能，纯新增特性
