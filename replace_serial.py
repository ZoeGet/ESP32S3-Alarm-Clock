#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
批量替换源文件中的 Serial.print 为 DEBUG_PRINT 宏
"""

import os
import re

# 需要处理的文件列表
files_to_process = [
    'src/AlarmTrigger.cpp',
    'src/NetworkAudioPlayer.cpp',
    'src/Speaker.cpp',
    'src/PressureSensor.cpp',
    'src/Clock.cpp',
    'src/Alarm.cpp',
]

# 模块映射
module_mapping = {
    'AlarmTrigger.cpp': 'DEBUG_MODULE_ALARM',
    'NetworkAudioPlayer.cpp': 'DEBUG_MODULE_AUDIO',
    'Speaker.cpp': 'DEBUG_MODULE_SPEAKER',
    'PressureSensor.cpp': 'DEBUG_MODULE_SENSOR',
    'Clock.cpp': 'DEBUG_MODULE_MAIN',
    'Alarm.cpp': 'DEBUG_MODULE_ALARM',
    'RemoteTaskSync.cpp': 'DEBUG_MODULE_NETWORK',
}

def replace_serial_with_debug(file_path):
    """替换文件中的 Serial.print 为 DEBUG_PRINT 宏"""
    
    if not os.path.exists(file_path):
        print(f"文件不存在：{file_path}")
        return
    
    # 读取文件
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 检查是否已包含 DebugHelper.h
    if '#include "DebugHelper.h"' not in content:
        # 在第一个 #include 后添加
        content = re.sub(
            r'(#include\s+".*?")',
            r'\1\n#include "DebugHelper.h"',
            content,
            count=1
        )
    
    # 获取文件名
    filename = os.path.basename(file_path)
    module_name = module_mapping.get(filename, 'DEBUG_MODULE_MAIN')
    
    # 替换 Serial.println()
    # 匹配 Serial.println("...") 或 Serial.println(String 变量)
    def replace_println(match):
        arg = match.group(1)
        # 检查是否是字符串字面量
        if arg.startswith('"'):
            # 字符串字面量，移除末尾的 \n
            if arg.endswith('\\n"'):
                arg = arg[:-3] + '"'
            return f'DEBUG_INFO({module_name}, %s, {arg})'
        else:
            # 变量或表达式
            return f'DEBUG_INFO({module_name}, "%s", {arg}.c_str())'
    
    content = re.sub(
        r'Serial\.println\(([^)]+)\)',
        replace_println,
        content
    )
    
    # 替换 Serial.print()
    # 匹配 Serial.print("...") 或 Serial.print(变量)
    def replace_print(match):
        arg = match.group(1)
        if arg.startswith('"'):
            return f'DEBUG_PRINTF({module_name}, %s, {arg})'
        else:
            return f'DEBUG_PRINTF({module_name}, "%s", {arg}.c_str())'
    
    content = re.sub(
        r'Serial\.print\(([^)]+)\)',
        replace_print,
        content
    )
    
    # 替换 Serial.printf()
    # Serial.printf("format", args...) -> DEBUG_PRINTF(module, "format", args...)
    def replace_printf(match):
        args = match.group(1)
        return f'DEBUG_PRINTF({module_name}, {args})'
    
    content = re.sub(
        r'Serial\.printf\(([^)]+(?:,[^)]+)*)\)',
        replace_printf,
        content
    )
    
    # 写回文件
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"已处理：{file_path}")

def main():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    
    for file_path in files_to_process:
        full_path = os.path.join(base_dir, file_path)
        replace_serial_with_debug(full_path)
    
    print("\n所有文件处理完成！")

if __name__ == '__main__':
    main()
