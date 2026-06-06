#include "Speaker.h"
#include <math.h>
#include "DebugHelper.h"

// 构造函数：初始化喇叭对象
Speaker::Speaker(uint8_t pin) 
    : _pin(pin)
    , _initialized(false)
    , _lastPlayTime(0)
    , _audioData(nullptr)
    , _audioDataSize(0)
    , _i2sInstalled(false) {}

// 析构函数：释放资源
Speaker::~Speaker() {
    if (_audioData) {
        delete[] _audioData;
    }
}

// 初始化喇叭
void Speaker::begin() {
    setupPWM();
    generateAudioData();
    _initialized = true;
    _i2sInstalled = false;  // 初始时不安装 I2S 驱动
}

void Speaker::setupPWM() {
    // 初始化 PWM 通道 0，1kHz，8 位分辨率
    ledcSetup(0, 1000, 8);
    // 绑定到 GPIO
    ledcAttachPin(_pin, 0);
    // 初始静音
    ledcWrite(0, 0);
    DEBUG_INFO(DEBUG_MODULE_SPEAKER, "Speaker PWM initialized on GPIO%d", _pin);
}

void Speaker::setupI2S() {
    // 如果 I2S 已经安装，先释放
    if (_i2sInstalled) {
        releaseI2S();
    }
    
    // 配置 I2S 参数
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    // 配置 I2S 引脚
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // 安装并配置 I2S 驱动（使用 I2S_NUM_1，避免与 NetworkAudioPlayer 冲突）
    esp_err_t err = i2s_driver_install(I2S_NUM_1, &i2s_config, 0, nullptr);
    if (err == ESP_OK) {
        i2s_set_pin(I2S_NUM_1, &pin_config);
        _i2sInstalled = true;
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker I2S installed (I2S_NUM_1)");
    } else {
        DEBUG_PRINTF(DEBUG_MODULE_SPEAKER, %s, "Speaker I2S install failed: ");
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, "%s", err.c_str());
    }
}

void Speaker::releaseI2S() {
    if (_i2sInstalled) {
        i2s_driver_uninstall(I2S_NUM_1);
        _i2sInstalled = false;
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker I2S released");
    }
}

void Speaker::generateAudioData() {
    // 计算1秒的采样点数
    _audioDataSize = SAMPLE_RATE * 1; // 1秒
    _audioData = new int16_t[_audioDataSize];
    
    // 生成800Hz正弦波
    const float frequency = 600.0f;
    const float amplitude = 32767.0f * 0.2f; // 20% amplitude
    
    for (size_t i = 0; i < _audioDataSize; i++) {
        float t = (float)i / SAMPLE_RATE;
        _audioData[i] = (int16_t)(amplitude * sin(2 * PI * frequency * t));
    }
    
    DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Audio data generated");
}

// 播放声音：指定频率和时长
void Speaker::playTone(uint32_t frequency, uint32_t duration) {
    if (!_initialized) {
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker not initialized");
        return;
    }
    
    // 检查最小间隔，防止短时间内重复播放
    unsigned long currentTime = millis();
    if (currentTime - _lastPlayTime < _minInterval) {
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker interval too short");
        return;
    }
    
    // 限制持续时间，最长200ms
    if (duration > 200) {
        duration = 200;
    }
    
    // 重新配置PWM频率
    ledcSetup(0, frequency, 8);
    // 设置低占空比（5%）
    ledcWrite(0, _maxDutyCycle);
    
    // 等待指定时间
    delay(duration);
    
    // 停止播放
    stop();
    
    // 更新上次播放时间
    _lastPlayTime = currentTime;
    DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker played tone: " + String(frequency) + "Hz for " + String(duration) + "ms");
}

// 播放 1 秒音频
void Speaker::playOneSecondTone() {
    if (!_initialized || !_audioData) {
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker not initialized or audio data not generated");
        return;
    }
    
    // 检查最小间隔，防止短时间内重复播放
    unsigned long currentTime = millis();
    if (currentTime - _lastPlayTime < _minInterval) {
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker interval too short");
        return;
    }
    
    // 使用 I2S 播放音频
    playI2SAudio();
    
    // 更新上次播放时间
    _lastPlayTime = currentTime;
    DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "Speaker played 1 second tone");
}

void Speaker::playI2SAudio() {
    if (!_audioData) {
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "[Speaker] 错误：音频数据为空");
        return;
    }
    
    // 临时安装 I2S 驱动
    if (!_i2sInstalled) {
        setupI2S();
    }
    
    if (!_i2sInstalled) {
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "[Speaker] I2S 驱动未安装，无法播放");
        return;
    }
    
    size_t total_bytes = _audioDataSize * sizeof(int16_t);
    size_t bytes_written = 0;
    
    DEBUG_PRINTF(DEBUG_MODULE_SPEAKER, %s, "[Speaker] 开始播放 I2S 音频，数据大小：");
    DEBUG_PRINTF(DEBUG_MODULE_SPEAKER, "%s", total_bytes.c_str());
    DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, " 字节");
    
    esp_err_t result = i2s_write(I2S_NUM_1, _audioData, total_bytes, &bytes_written, portMAX_DELAY);
    
    DEBUG_PRINTF(DEBUG_MODULE_SPEAKER, %s, "[Speaker] I2S 写入结果：");
    DEBUG_PRINTF(DEBUG_MODULE_SPEAKER, "%s", result == ESP_OK ? "成功" : "失败".c_str());
    DEBUG_PRINTF(DEBUG_MODULE_SPEAKER, %s, ", 写入字节数：");
    DEBUG_INFO(DEBUG_MODULE_SPEAKER, "%s", bytes_written.c_str());
    
    if (result != ESP_OK) {
        DEBUG_PRINTF(DEBUG_MODULE_SPEAKER, %s, "[Speaker] I2S 错误代码：");
        DEBUG_INFO(DEBUG_MODULE_SPEAKER, "%s", result.c_str());
    }
    
    // 等待播放完成
    delay(1000);
    
    // 清除 I2S 缓冲区
    i2s_zero_dma_buffer(I2S_NUM_1);
    
    // 释放 I2S 驱动，让出给 NetworkAudioPlayer
    releaseI2S();
    
    DEBUG_INFO(DEBUG_MODULE_SPEAKER, %s, "[Speaker] I2S 音频播放完成");
}

// 停止播放
void Speaker::stop() {
    if (_initialized) {
        ledcWrite(0, 0);
        // 清除 I2S 缓冲区
        if (_i2sInstalled) {
            i2s_zero_dma_buffer(I2S_NUM_1);
        }
    }
}

// 检查是否已初始化
bool Speaker::isInitialized() const {
    return _initialized;
}
