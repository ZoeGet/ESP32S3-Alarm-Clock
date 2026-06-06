// 硬件测试：直接测试 MAX98357A 模块
// 将此代码添加到 main.cpp 的 loop() 中进行测试

#include <driver/i2s.h>

// 生成 440Hz 正弦波（A 音）
void testMAX98357A() {
    Serial.println("[TEST] 开始测试 MAX98357A 硬件...");
    
    // 配置 I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    // 安装 I2S 驱动
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, nullptr);
    Serial.print("[TEST] I2S 驱动安装：");
    Serial.println(err == ESP_OK ? "成功" : "失败");
    
    if (err != ESP_OK) {
        return;
    }
    
    // 配置引脚
    i2s_pin_config_t pin_config = {
        .bck_io_num = 47,   // BCLK
        .ws_io_num = 21,    // LRC
        .data_out_num = 48, // DIN
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    Serial.print("[TEST] I2S 引脚配置：");
    Serial.println(err == ESP_OK ? "成功" : "失败");
    
    // 生成 1 秒的 440Hz 正弦波
    const int sample_rate = 44100;
    const int duration = 1; // 秒
    const int num_samples = sample_rate * duration;
    int16_t* samples = new int16_t[num_samples];
    
    const float frequency = 440.0f; // A 音
    const float amplitude = 32767.0f * 0.5f; // 50% 音量
    
    for (int i = 0; i < num_samples; i++) {
        float t = (float)i / sample_rate;
        samples[i] = (int16_t)(amplitude * sin(2 * PI * frequency * t));
    }
    
    Serial.println("[TEST] 开始播放 440Hz 正弦波...");
    
    // 播放
    size_t bytes_written;
    i2s_write(I2S_NUM_0, samples, num_samples * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    
    Serial.print("[TEST] 播放完成，写入字节数：");
    Serial.println(bytes_written);
    
    // 清理
    delete[] samples;
    i2s_driver_uninstall(I2S_NUM_0);
    
    Serial.println("[TEST] MAX98357A 测试完成");
}

// 在 loop() 中调用（只执行一次）
// 在 setup() 末尾添加：
// testMAX98357A();
