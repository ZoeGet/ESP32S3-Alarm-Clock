#include "NetworkAudioPlayer.h"
#include <SPIFFS.h>
#include "DebugHelper.h"

// 静态回调函数实现
void NetworkAudioPlayer::audioInfoCallback(const char* info) {
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioInfo] %s", info);
}

// 音频元数据回调
static void audioMetadataCallback(const char* type, int32_t value) {
    if (type == nullptr) return;
    
    String typeStr = String(type);
    if (typeStr == "AUDIO_INFO") {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioMeta] 音频信息：%d", value);
    } else if (typeStr == "AUDIO_BITRATE") {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioMeta] 比特率：%d bps", value);
    } else if (typeStr == "AUDIO_SAMPLE_RATE") {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioMeta] 采样率：%d Hz", value);
    } else if (typeStr == "AUDIO_CHANNELS") {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioMeta] 声道数：%d", value);
    } else if (typeStr == "AUDIO_BITS_PER_SAMPLE") {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioMeta] 位深度：%d bit", value);
    } else if (typeStr == "AUDIO_DURATION") {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioMeta] 时长：%d 秒", value);
    }
}

NetworkAudioPlayer::NetworkAudioPlayer() 
    : m_audio(nullptr)
    , m_wifiClient(nullptr)
    , m_state(AUDIO_IDLE)
    , m_volume(20)  // 默认音量
    , m_playStartTime(0)
    , m_isInitialized(false) {
}

bool NetworkAudioPlayer::begin(const String& deviceId) {
    if (m_isInitialized) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 已经初始化");
        return true;
    }
    
    // 保存设备 ID
    m_deviceId = deviceId;
    m_deviceId.trim();  // 去除空格
    m_deviceId.toUpperCase();  // 转换为大写
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 初始化音频播放器...");
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 设备 ID: %s", m_deviceId.c_str());
    
    // 创建 WiFi 客户端
    m_wifiClient = new WiFiClientSecure();
    if (!m_wifiClient) {
        updateState(AUDIO_ERROR, "无法创建 WiFi 客户端");
        return false;
    }
    
    // 忽略证书验证（用于开发）- 这对 connecttohost 也生效
    m_wifiClient->setInsecure();
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 已设置忽略证书验证");
    
    // 创建 Audio 对象
    m_audio = new Audio(*m_wifiClient);
    if (!m_audio) {
        updateState(AUDIO_ERROR, "无法创建 Audio 对象");
        delete m_wifiClient;
        m_wifiClient = nullptr;
        return false;
    }
    
    // 短暂延迟，确保 Audio 对象完全初始化（避免 I2S 注册失败）
    delay(100);
    
    // 配置 I2S 引脚
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 配置 I2S 引脚...");
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] BCLK: GPIO%d", I2S_BCLK);
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] LRC: GPIO%d", I2S_LRC);
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] DIN: GPIO%d", I2S_DIN);
    
    // 设置 I2S 引脚（ESP32-audioI2S 库的 API）
    // 注意：ESP32-audioI2S 使用 setPinout(BCLK, LRC, DIN) 顺序
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 调用 setPinout()...");
    bool pinoutSet = m_audio->setPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] setPinout() 返回值：%s", pinoutSet ? "true" : "false");
    
    if (pinoutSet) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] I2S 引脚配置成功");
    } else {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] I2S 引脚配置失败！");
        updateState(AUDIO_ERROR, "I2S 引脚配置失败");
        delete m_audio;
        m_audio = nullptr;
        delete m_wifiClient;
        m_wifiClient = nullptr;
        return false;
    }
    
    // 设置初始音量
    m_audio->setVolume(m_volume);
    
    // 设置单声道输出（MAX98357A 是单声道放大器）
    m_audio->forceMono(true);
    
    m_isInitialized = true;
    updateState(AUDIO_IDLE);
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 初始化完成");
    return true;
}

void NetworkAudioPlayer::end() {
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 释放资源...");
    
    // 停止播放
    stopAudio();
    
    // 删除 Audio 对象
    if (m_audio) {
        delete m_audio;
        m_audio = nullptr;
    }
    
    // 删除 WiFi 客户端
    if (m_wifiClient) {
        delete m_wifiClient;
        m_wifiClient = nullptr;
    }
    
    m_isInitialized = false;
    updateState(AUDIO_IDLE);
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 资源已释放");
}

bool NetworkAudioPlayer::playAudioByUrl(const String& audioUrl) {
    if (!m_isInitialized) {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 错误：未初始化");
        return false;
    }
    
    if (m_state == AUDIO_PLAYING) {
        DEBUG_WARNING(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 警告：正在播放，先停止");
        stopAudio();
    }
    
    // 检查 WiFi 连接
    if (WiFi.status() != WL_CONNECTED) {
        updateState(AUDIO_ERROR, "WiFi 未连接");
        return false;
    }
    
    // 检查 URL 是否有效
    if (audioUrl.length() == 0) {
        updateState(AUDIO_ERROR, "音频 URL 为空");
        return false;
    }
    
    // 将 HTTPS URL 转换为 HTTP
    String httpUrl = audioUrl;
    httpUrl.replace("https://", "http://");
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 下载音频 URL: %s", httpUrl.c_str());
    
    // 先下载音频文件到 SPIFFS
    if (!downloadAudioFile(httpUrl)) {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 下载失败！");
        return false;
    }
    
    // 播放本地文件
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 开始播放本地文件...");
    updateState(AUDIO_PLAYING);
    
    bool connected = m_audio->connecttoFS(SPIFFS, "/alarm.mp3");
    
    if (!connected) {
        updateState(AUDIO_ERROR, "播放失败");
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 播放失败！");
        return false;
    }
    
    // 等待音频解码器初始化
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 等待音频解码器初始化...");
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) {
        m_audio->loop();
        delay(10);
        
        if (m_audio->isRunning()) {
            DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 音频解码器已启动");
            break;
        }
    }
    
    // 检查音频播放状态
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 音频解码器状态：%s", m_audio->isRunning() ? "运行中" : "未运行");
    
    // 记录播放开始时间
    m_playStartTime = millis();
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 播放已开始");
    return true;
}

String NetworkAudioPlayer::getAudioPlayUrl(const String& fileId) {
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 获取音频播放 URL，file_id=%s", fileId.c_str());
    
    // 检查 WiFi 连接
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] WiFi 未连接");
        return "";
    }
    
    // 构建获取音频 URL 的请求
    String baseUrl = String(AUDIO_SERVER_URL);
    String url = baseUrl + "/api/v1/deviceControl/audio?device_id=" + m_deviceId + "&file_id=" + fileId;
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 请求 URL: %s", url.c_str());
    
    // 创建 HTTP 客户端
    HTTPClient http;
    http.begin(*m_wifiClient, url);
    
    // 设置不自动跟随重定向，我们需要获取 Location 头
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    
    // 发起 GET 请求
    int httpResponseCode = http.GET();
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] HTTP 响应码：%d", httpResponseCode);
    
    if (httpResponseCode != 302) {
        DEBUG_PRINTF(DEBUG_MODULE_AUDIO, %s, "[NetworkAudioPlayer] 错误：期望 302 重定向，实际响应码：");
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "%s", httpResponseCode.c_str());
        http.end();
        return "";
    }
    
    // 获取 Location 头（实际音频文件地址）
    String audioUrl = http.header("Location");
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] Location 头：%s", audioUrl.c_str());
    
    http.end();
    
    if (audioUrl.length() == 0) {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 错误：Location 头为空");
        return "";
    }
    
    // 检查 Location URL 是否可访问
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 测试 Location URL...");
    HTTPClient testHttp;
    testHttp.begin(*m_wifiClient, audioUrl);
    testHttp.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    int testCode = testHttp.GET();
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] Location URL 响应码：%d", testCode);
    testHttp.end();
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 获取到音频 URL: %s", audioUrl.c_str());
    
    return audioUrl;
}

void NetworkAudioPlayer::stopAudio() {
    if (!m_isInitialized) {
        return;
    }
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 停止播放...");
    
    // 停止音频播放
    if (m_audio) {
        m_audio->stopSong();
    }
    
    // 重置状态
    updateState(AUDIO_IDLE);
    m_playStartTime = 0;
}

bool NetworkAudioPlayer::isPlaying() {
    return (m_state == AUDIO_PLAYING);
}

AudioPlayState NetworkAudioPlayer::getState() {
    return m_state;
}

String NetworkAudioPlayer::getLastError() {
    return m_lastError;
}

void NetworkAudioPlayer::setVolume(uint8_t volume) {
    m_volume = volume;
    
    if (m_audio && m_isInitialized) {
        m_audio->setVolume(m_volume);
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 音量已设置：%d", m_volume);
    }
}

uint8_t NetworkAudioPlayer::getVolume() {
    return m_volume;
}

void NetworkAudioPlayer::updateState(AudioPlayState newState, const String& error) {
    m_state = newState;
    
    if (error.length() > 0) {
        m_lastError = error;
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 错误：%s", error.c_str());
    }
    
    // 状态变化时打印日志
    switch (newState) {
        case AUDIO_IDLE:
            DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 状态：空闲");
            break;
        case AUDIO_CONNECTING:
            DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 状态：连接中...");
            break;
        case AUDIO_PLAYING:
            DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 状态：播放中");
            break;
        case AUDIO_COMPLETED:
            DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 状态：播放完成");
            break;
        case AUDIO_ERROR:
            DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 状态：错误");
            break;
    }
}

// 需要在 loop 中调用的更新函数
void NetworkAudioPlayer::loop() {
    if (!m_isInitialized || m_state != AUDIO_PLAYING) {
        return;
    }
    
    // 处理音频流
    if (m_audio) {
        m_audio->loop();
        
        // 检查是否真的在播放
        bool isRunning = m_audio->isRunning();
        
        // 每 1 秒打印一次播放状态
        static unsigned long lastStatusPrint = 0;
        if (millis() - lastStatusPrint > 1000) {
            DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 播放状态 - 运行中：%s", isRunning ? "是" : "否");
            
            lastStatusPrint = millis();
            
            // 如果停止运行，记录错误
            if (!isRunning && m_state == AUDIO_PLAYING) {
                DEBUG_WARNING(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 警告：播放意外停止！");
            }
        }
    }
    
    // 检查播放超时
    if (millis() - m_playStartTime > AUDIO_PLAY_TIMEOUT_MS) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 播放超时，自动停止");
        stopAudio();
    }
}

// 下载音频文件到 SPIFFS
bool NetworkAudioPlayer::downloadAudioFile(const String& url) {
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] === 开始下载音频文件 ===");
    
    // 初始化 SPIFFS，如果失败则格式化
    if (!SPIFFS.begin(true)) {
        DEBUG_WARNING(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] SPIFFS 挂载失败，尝试格式化...");
        SPIFFS.format();
        if (!SPIFFS.begin(true)) {
            DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] SPIFFS 格式化失败");
            return false;
        }
    }
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] SPIFFS 初始化成功");
    
    // 创建 HTTP 客户端
    HTTPClient http;
    http.begin(url);
    http.setTimeout(10000);  // 设置 10 秒超时
    
    // 发起 GET 请求
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 发起 HTTP 请求...");
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] HTTP 请求失败，响应码：%d", httpCode);
        http.end();
        return false;
    }
    
    // 获取文件大小
    int fileSize = http.getSize();
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 文件大小：%d 字节", fileSize);
    
    if (fileSize <= 0) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 文件大小为 0 或未知");
        // 继续下载，但限制最大大小
        fileSize = MAX_AUDIO_SIZE;
    }
    
    if (fileSize > MAX_AUDIO_SIZE) {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 文件过大（%d > %d）", fileSize, MAX_AUDIO_SIZE);
        http.end();
        return false;
    }
    
    // 删除旧文件
    if (SPIFFS.exists("/alarm.mp3")) {
        SPIFFS.remove("/alarm.mp3");
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 已删除旧文件");
    }
    
    // 打开文件
    File file = SPIFFS.open("/alarm.mp3", FILE_WRITE);
    if (!file) {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 无法创建文件");
        http.end();
        return false;
    }
    
    // 获取 WiFi 客户端
    WiFiClient* stream = http.getStreamPtr();
    
    // 下载文件
    DEBUG_INFO(DEBUG_MODULE_AUDIO, %s, "[NetworkAudioPlayer] 开始下载...");
    int downloaded = 0;
    unsigned long lastProgressTime = millis();
    
    while (http.connected() && downloaded < fileSize) {
        size_t size = stream->available();
        if (size) {
            // 读取数据
            uint8_t* buffer = new uint8_t[size];
            int bytesRead = stream->readBytes(buffer, size);
            file.write(buffer, bytesRead);
            delete[] buffer;
            
            downloaded += bytesRead;
            
            // 每 1 秒打印一次进度
            if (millis() - lastProgressTime > 1000) {
                DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 下载进度：%d/%d 字节", downloaded, fileSize);
                lastProgressTime = millis();
            }
        }
        delay(1);
    }
    
    file.close();
    http.end();
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 下载完成，总大小：%d 字节", downloaded);
    
    // 验证文件是否存在
    if (SPIFFS.exists("/alarm.mp3")) {
        File verifyFile = SPIFFS.open("/alarm.mp3", FILE_READ);
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 文件验证成功，实际大小：%d", verifyFile.size());
        verifyFile.close();
        return true;
    } else {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 文件验证失败");
        return false;
    }
}

// 测试 MAX98357A 硬件
void NetworkAudioPlayer::testHardware() {
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] === 开始测试 MAX98357A 硬件 ===");
    
    // 检查 WiFi 连接
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] WiFi 未连接，跳过测试");
        return;
    }
    
    // 使用一个已知的在线音频流进行测试（冰岛的在线电台）
    String testUrl = "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service";
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 播放测试音频流：BBC World Service");
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] URL: %s", testUrl.c_str());
    
    bool connected = m_audio->connecttohost(testUrl.c_str());
    
    if (connected) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 测试音频流连接成功！");
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 如果你听到声音，说明 MAX98357A 硬件正常");
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 等待 10 秒后停止测试...");
        
        // 播放 10 秒
        unsigned long startTime = millis();
        while (millis() - startTime < 10000) {
            m_audio->loop();
            delay(10);
        }
        
        m_audio->stopSong();
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 测试完成");
    } else {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] 测试音频流连接失败！");
    }
    
    DEBUG_INFO(DEBUG_MODULE_AUDIO, "[NetworkAudioPlayer] === MAX98357A 硬件测试结束 ===");
}
