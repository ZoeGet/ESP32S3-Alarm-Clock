#include "DebugHelper.h"
#include <stdarg.h>

// 静态成员初始化
int DebugHelper::s_debugLevel = DEBUG_LEVEL_NONE;
uint16_t DebugHelper::s_debugModules = DEBUG_MODULE_NONE;
bool DebugHelper::s_debugEnabled = false;
char DebugHelper::s_inputBuffer[64] = {0};
int DebugHelper::s_inputIndex = 0;

// 创建全局调试实例
static DebugHelper g_debugInstance;
DebugHelper& Debug = g_debugInstance;

void DebugHelper::begin(int initialLevel, uint16_t modules) {
    s_debugLevel = initialLevel;
    s_debugModules = modules;
    s_debugEnabled = (initialLevel > DEBUG_LEVEL_NONE);
    s_inputIndex = 0;
    
    if (s_debugEnabled) {
        Serial.println("[Debug] 调试系统已启动");
        Serial.printf("[Debug] 调试级别：%d\n", s_debugLevel);
        Serial.printf("[Debug] 调试模块：0x%04X\n", s_debugModules);
    }
}

void DebugHelper::setDebugLevel(int level) {
    s_debugLevel = level;
    if (level > DEBUG_LEVEL_NONE) {
        s_debugEnabled = true;
        Serial.println("[Debug] 调试模式已开启");
    } else {
        s_debugEnabled = false;
        Serial.println("[Debug] 调试模式已关闭");
    }
}

int DebugHelper::getDebugLevel() {
    return s_debugLevel;
}

void DebugHelper::setDebugModule(uint16_t module, bool enable) {
    if (enable) {
        s_debugModules |= module;
    } else {
        s_debugModules &= ~module;
    }
}

bool DebugHelper::isModuleEnabled(uint16_t module) {
    return (s_debugModules & module) != 0;
}

void DebugHelper::enableDebug() {
    s_debugEnabled = true;
    Serial.println("[Debug] 调试模式已启用");
}

void DebugHelper::disableDebug() {
    s_debugEnabled = false;
    Serial.println("[Debug] 调试模式已禁用");
}

bool DebugHelper::isDebugEnabled() {
    return s_debugEnabled;
}

void DebugHelper::handleSerialCommand() {
    // 读取串口数据
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        // 处理回车键
        if (c == '\n' || c == '\r') {
            if (s_inputIndex > 0) {
                s_inputBuffer[s_inputIndex] = '\0';
                
                // 检查命令
                String command = String(s_inputBuffer);
                command.trim();
                
                if (command == "debug looklook") {
                    enableDebug();
                    setDebugLevel(DEBUG_LEVEL_DEBUG);
                    setDebugModule(DEBUG_MODULE_ALL, true);
                    Serial.println("[Debug] 已开启所有调试信息");
                } else if (command == "debug off") {
                    disableDebug();
                    setDebugLevel(DEBUG_LEVEL_NONE);
                    Serial.println("[Debug] 已关闭所有调试信息");
                } else if (command == "debug help") {
                    Serial.println("\n=== 调试命令帮助 ===");
                    Serial.println("debug looklook - 开启所有调试信息");
                    Serial.println("debug off - 关闭所有调试信息");
                    Serial.println("debug level <0-4> - 设置调试级别");
                    Serial.println("  0: 关闭所有");
                    Serial.println("  1: 仅错误");
                    Serial.println("  2: 错误 + 警告");
                    Serial.println("  3: 错误 + 警告 + 信息");
                    Serial.println("  4: 所有调试信息");
                    Serial.println("debug module <name> <on/off> - 控制模块调试");
                    Serial.println("  模块名：main, network, audio, alarm, sensor, display, speaker, timer");
                    Serial.println("debug status - 显示当前调试状态");
                    Serial.println("=====================\n");
                } else if (command.startsWith("debug level ")) {
                    int level = command.substring(12).toInt();
                    if (level >= 0 && level <= 4) {
                        setDebugLevel(level);
                        Serial.printf("[Debug] 调试级别已设置为：%d\n", level);
                    } else {
                        Serial.println("[Debug] 错误：级别必须在 0-4 之间");
                    }
                } else if (command.startsWith("debug module ")) {
                    // 解析模块命令
                    int firstSpace = command.indexOf(' ', 13);
                    if (firstSpace > 0) {
                        String moduleName = command.substring(13, firstSpace);
                        String state = command.substring(firstSpace + 1);
                        
                        uint16_t module = DEBUG_MODULE_NONE;
                        if (moduleName == "main") module = DEBUG_MODULE_MAIN;
                        else if (moduleName == "network") module = DEBUG_MODULE_NETWORK;
                        else if (moduleName == "audio") module = DEBUG_MODULE_AUDIO;
                        else if (moduleName == "alarm") module = DEBUG_MODULE_ALARM;
                        else if (moduleName == "sensor") module = DEBUG_MODULE_SENSOR;
                        else if (moduleName == "display") module = DEBUG_MODULE_DISPLAY;
                        else if (moduleName == "speaker") module = DEBUG_MODULE_SPEAKER;
                        else if (moduleName == "timer") module = DEBUG_MODULE_TIMER;
                        
                        if (module != DEBUG_MODULE_NONE) {
                            bool enable = (state == "on" || state == "1");
                            setDebugModule(module, enable);
                            Serial.printf("[Debug] 模块 %s 已%s\n", 
                                moduleName.c_str(), enable ? "开启" : "关闭");
                        } else {
                            Serial.printf("[Debug] 错误：未知模块 '%s'\n", moduleName.c_str());
                        }
                    }
                } else if (command == "debug status") {
                    Serial.println("\n=== 调试状态 ===");
                    Serial.printf("调试开关：%s\n", s_debugEnabled ? "开启" : "关闭");
                    Serial.printf("调试级别：%d\n", s_debugLevel);
                    Serial.printf("调试模块：0x%04X\n", s_debugModules);
                    Serial.println("================\n");
                } else if (command.length() > 0) {
                    Serial.printf("[Debug] 未知命令：%s\n", command.c_str());
                    Serial.println("输入 'debug help' 查看帮助");
                }
                
                // 清空缓冲区
                s_inputIndex = 0;
                memset(s_inputBuffer, 0, sizeof(s_inputBuffer));
            }
        } else {
            // 添加到缓冲区
            if (s_inputIndex < sizeof(s_inputBuffer) - 1) {
                s_inputBuffer[s_inputIndex++] = c;
            }
        }
    }
}

void DebugHelper::print(int level, uint16_t module, const char* format, ...) {
    if (!s_debugEnabled || level > s_debugLevel || !isModuleEnabled(module)) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    // 使用缓冲区格式化输出
    char buffer[256];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    
    if (len > 0) {
        Serial.write(buffer, len);
    }
    
    va_end(args);
}

void DebugHelper::println(int level, uint16_t module, const char* format, ...) {
    if (!s_debugEnabled || level > s_debugLevel || !isModuleEnabled(module)) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    // 使用缓冲区格式化输出
    char buffer[256];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    
    if (len > 0) {
        Serial.write(buffer, len);
    }
    Serial.println();
    
    va_end(args);
}

void DebugHelper::printf(int level, uint16_t module, const char* format, ...) {
    if (!s_debugEnabled || level > s_debugLevel || !isModuleEnabled(module)) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    // 使用缓冲区格式化输出
    char buffer[256];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    
    if (len > 0) {
        Serial.write(buffer, len);
    }
    
    va_end(args);
}
