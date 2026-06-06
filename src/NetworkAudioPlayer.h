#ifndef NETWORK_AUDIO_PLAYER_H
#define NETWORK_AUDIO_PLAYER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Audio.h>  // ESP32-audioI2S 库

// I2S 引脚配置（MAX98357A）
#define I2S_BCLK 15      // 位时钟
#define I2S_LRC 16       // 左右声道时钟
#define I2S_DIN 7        // 数据输入

// 播放配置
#define AUDIO_PLAY_TIMEOUT_MS 30000  // 30 秒播放超时
#define AUDIO_CONNECT_TIMEOUT_MS 10000  // 10 秒连接超时
#define MAX_AUDIO_SIZE 500000  // 最大音频文件大小（500KB）

// 播放状态枚举
enum AudioPlayState {
    AUDIO_IDLE = 0,
    AUDIO_CONNECTING,
    AUDIO_PLAYING,
    AUDIO_COMPLETED,
    AUDIO_ERROR
};

// 网络音频播放器类
class NetworkAudioPlayer {
public:
    // 构造函数
    NetworkAudioPlayer();
    
    // 初始化音频播放器
    bool begin(const String& deviceId = "");
    
    // 释放资源
    void end();
    
    // 播放音频文件（通过 file_id，已废弃）
    // bool playAudio(const String& fileId);
    
    // 播放音频文件（直接使用 audio_url）
    bool playAudioByUrl(const String& audioUrl);
    
    // 停止播放
    void stopAudio();
    
    // 检查是否正在播放
    bool isPlaying();
    
    // 获取播放状态
    AudioPlayState getState();
    
    // 获取错误信息
    String getLastError();
    
    // 设置音量 (0-255)
    void setVolume(uint8_t volume);
    
    // 获取当前音量
    uint8_t getVolume();
    
    // 音频流处理（需在 loop 中调用）
    void loop();
    
    // 测试 MAX98357A 硬件
    void testHardware();
    
    // 下载音频文件到 SPIFFS
    bool downloadAudioFile(const String& url);

private:
    Audio* m_audio;  // ESP32-audioI2S 对象
    WiFiClientSecure* m_wifiClient;  // WiFi 客户端
    String m_deviceId;  // 设备 ID
    AudioPlayState m_state;  // 当前状态
    String m_lastError;  // 最后错误信息
    uint8_t m_volume;  // 音量 (0-255)
    unsigned long m_playStartTime;  // 播放开始时间
    bool m_isInitialized;  // 是否已初始化
    
    // 获取音频播放 URL（通过 302 重定向）
    String getAudioPlayUrl(const String& fileId);
    
    // 内部状态更新
    void updateState(AudioPlayState newState, const String& error = "");
    
    // 音频事件回调（静态）
    static void audioInfoCallback(const char* info);
};

#endif
