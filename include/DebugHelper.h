#ifndef DEBUG_HELPER_H
#define DEBUG_HELPER_H

#include <Arduino.h>

// 调试级别定义
#define DEBUG_LEVEL_NONE    0    // 关闭所有调试信息
#define DEBUG_LEVEL_ERROR   1    // 仅错误信息
#define DEBUG_LEVEL_WARNING 2    // 错误 + 警告
#define DEBUG_LEVEL_INFO    3    // 错误 + 警告 + 信息
#define DEBUG_LEVEL_DEBUG   4    // 所有调试信息

// 模块标志定义
#define DEBUG_MODULE_NONE       0x0000
#define DEBUG_MODULE_MAIN       0x0001
#define DEBUG_MODULE_NETWORK    0x0002
#define DEBUG_MODULE_AUDIO      0x0004
#define DEBUG_MODULE_ALARM      0x0008
#define DEBUG_MODULE_SENSOR     0x0010
#define DEBUG_MODULE_DISPLAY    0x0020
#define DEBUG_MODULE_SPEAKER    0x0040
#define DEBUG_MODULE_TIMER      0x0080
#define DEBUG_MODULE_ALL        0xFFFF

// 调试管理类
class DebugHelper {
private:
    static int s_debugLevel;              // 当前调试级别
    static uint16_t s_debugModules;       // 启用的调试模块
    static bool s_debugEnabled;           // 调试总开关
    static char s_inputBuffer[64];        // 串口输入缓冲区
    static int s_inputIndex;              // 输入缓冲区索引
    
public:
    // 初始化
    static void begin(int initialLevel = DEBUG_LEVEL_NONE, uint16_t modules = DEBUG_MODULE_ALL);
    
    // 设置调试级别
    static void setDebugLevel(int level);
    static int getDebugLevel();
    
    // 设置调试模块
    static void setDebugModule(uint16_t module, bool enable);
    static bool isModuleEnabled(uint16_t module);
    
    // 启用/禁用调试
    static void enableDebug();
    static void disableDebug();
    static bool isDebugEnabled();
    
    // 处理串口命令（需要在 loop 中调用）
    static void handleSerialCommand();
    
    // 打印调试信息（带模块和级别）
    static void print(int level, uint16_t module, const char* format, ...);
    static void println(int level, uint16_t module, const char* format, ...);
    static void printf(int level, uint16_t module, const char* format, ...);
};

// 调试宏定义
#ifdef ENABLE_DEBUG

#define DEBUG_PRINT(module, ...) \
    do { \
        if (DebugHelper::isDebugEnabled() && DebugHelper::isModuleEnabled(module)) { \
            DebugHelper::print(DEBUG_LEVEL_DEBUG, module, __VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_PRINTLN(module, ...) \
    do { \
        if (DebugHelper::isDebugEnabled() && DebugHelper::isModuleEnabled(module)) { \
            DebugHelper::println(DEBUG_LEVEL_DEBUG, module, __VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_PRINTF(module, ...) \
    do { \
        if (DebugHelper::isDebugEnabled() && DebugHelper::isModuleEnabled(module)) { \
            DebugHelper::printf(DEBUG_LEVEL_DEBUG, module, __VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_INFO(module, ...) \
    do { \
        if (DebugHelper::isDebugEnabled() && DebugHelper::isModuleEnabled(module)) { \
            DebugHelper::println(DEBUG_LEVEL_INFO, module, __VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_WARNING(module, ...) \
    do { \
        if (DebugHelper::isDebugEnabled() && DebugHelper::isModuleEnabled(module)) { \
            DebugHelper::println(DEBUG_LEVEL_WARNING, module, __VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_ERROR(module, ...) \
    do { \
        if (DebugHelper::isDebugEnabled() && DebugHelper::isModuleEnabled(module)) { \
            DebugHelper::println(DEBUG_LEVEL_ERROR, module, __VA_ARGS__); \
        } \
    } while(0)

#else

#define DEBUG_PRINT(module, ...)
#define DEBUG_PRINTLN(module, ...)
#define DEBUG_PRINTF(module, ...)
#define DEBUG_INFO(module, ...)
#define DEBUG_WARNING(module, ...)
#define DEBUG_ERROR(module, ...)

#endif

// 全局调试实例
extern DebugHelper& Debug;

#endif // DEBUG_HELPER_H
