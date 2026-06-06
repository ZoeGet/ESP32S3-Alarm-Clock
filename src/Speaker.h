#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include <driver/i2s.h>

#define SPEAKER_PIN 5   // 喇叭引脚

// I2S 引脚定义
#define I2S_LRC 16
#define I2S_BCLK 15
#define I2S_DIN 7

// 音频参数
#define SAMPLE_RATE 8000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define CHANNELS 1

// 喇叭类：控制喇叭播放声音
class Speaker {
public:
    Speaker(uint8_t pin = 25);  // 构造函数：初始化喇叭对象，默认引脚 25
    ~Speaker();  // 析构函数：释放资源
    
    void begin();  // 初始化喇叭（只初始化 PWM）
    
    // 播放指定频率和时长的声音
    void playTone(uint32_t frequency, uint32_t duration);
    
    // 播放 1 秒音频
    void playOneSecondTone();
    
    // 使用 PWM 播放蜂鸣声
    void playPWMBeep();
    
    // 停止播放
    void stop();
    
    // 检查是否已初始化
    bool isInitialized() const;
    
private:
    uint8_t _pin;                  // 喇叭引脚
    bool _initialized;             // 初始化状态
    const uint8_t _maxDutyCycle = 20;  // 最大占空比（5%）
    const uint32_t _minInterval = 1000;  // 1 秒最小间隔
    unsigned long _lastPlayTime;    // 上次播放时间
    int16_t *_audioData;           // 音频数据
    size_t _audioDataSize;         // 音频数据大小
    bool _i2sInstalled;            // I2S 驱动是否已安装
    
    // 设置 PWM
    void setupPWM();
    
    // 设置 I2S（临时）
    void setupI2S();
    
    // 释放 I2S
    void releaseI2S();
    
    // 生成音频数据
    void generateAudioData();
    
    // 通过 I2S 播放音频
    void playI2SAudio();
};

#endif // SPEAKER_H
