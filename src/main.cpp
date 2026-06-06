#include <Arduino.h>
#include <WiFiManager.h>
#include "DebugHelper.h"
#include "Clock.h"
#include "OLED_Display.h"
#include "Timer.h"
#include "LED.h"
#include "Speaker.h"
#include "Alarm.h"
#include "PressureSensor.h"
#include "DeviceMAC.h"
#include "RemoteTaskSync.h"
#include "AlarmTrigger.h"
#include "NetworkAudioPlayer.h"
#include <ArduinoJson.h>


// 功能类实例
Clock rtcClock;        // 时钟对象
OLED_Display oled;     // OLED 显示对象
Timer timer;           // 定时器对象
LED led(LED_PIN);      // LED 对象
Speaker speaker(SPEAKER_PIN);  // 喇叭对象
Alarm timeAlarm(speaker, oled);  // 闹钟对象
PressureSensor pressureSensor;  // 压力传感器对象
NetworkAudioPlayer audioPlayer;  // 网络音频播放器对象

// 压力触发状态
bool lastPressureState = false;
bool currentPressureState = false;

// WiFiManager 实例
WiFiManager wifiManager;

/*
 * ========== IO0 长按重新配网（已整体注释禁用；需要时删掉本块首尾的 / * 和 * / 即可恢复）==========
 *
// IO0 按键相关
const int BUTTON_PIN = 0;  // IO0 引脚
bool lastButtonState = HIGH;  // 上一次按键状态（未按下为高电平）
bool currentButtonState = HIGH;  // 当前按键状态
unsigned long buttonPressStartTime = 0;  // 按键按下开始时间
const int BUTTON_PRESS_DURATION = 2000;  // 按键按下持续时间阈值（2 秒）
bool isReconfigMode = false;  // 是否进入重新配网模式

// 重新配网函数
void startReconfigWiFi() {
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "开始重新配网...");
    oled.showWiFiConfig();

    // 断开当前 WiFi 连接
    WiFi.disconnect(true);
    delay(500);

    // 启动配置门户
    wifiManager.startConfigPortal("ESP32-Clock");

    // 配网完成后，如果连接成功则显示 IP
    if (WiFi.status() == WL_CONNECTED) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "重新配网成功!");
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "IP 地址：%s", WiFi.localIP().toString().c_str());
        oled.showWiFiConnected(WiFi.localIP().toString().c_str());
        delay(500);
    } else {
        DEBUG_WARNING(DEBUG_MODULE_NETWORK, "重新配网失败或超时");
        oled.showWiFiConfig();
        delay(500);
    }

    isReconfigMode = false;
}
 * ========== IO0 长按重新配网 结束 ==========
 */

// 闹钟触发回调函数
void onAlarmTriggered() {
    DEBUG_INFO(DEBUG_MODULE_ALARM, "[Callback] 闹钟触发回调执行中...");
    
    // 获取当前触发的 task_id
    String taskId = AlarmTrigger::getCurrentTriggeredTaskId();
    
    // 根据 task_id 查找对应的 audio_url
    String audioUrl = AlarmTrigger::getAudioUrlByTaskId(taskId);
    
    if (audioUrl.length() == 0) {
        // audio_url 为空或不存在，播放 1 秒蜂鸣声
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[Callback] audio_url 为空，播放 1 秒蜂鸣声");
        speaker.playOneSecondTone();
    } else {
        // audio_url 存在，播放网络音频
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[Callback] audio_url: %s", audioUrl.c_str());
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[Callback] 播放网络音频（最长 30 秒）");
        
        // 使用网络音频播放器播放
        if (audioPlayer.playAudioByUrl(audioUrl)) {
            DEBUG_INFO(DEBUG_MODULE_AUDIO, "[Callback] 音频播放已开始");
        } else {
            // 播放失败，降级为蜂鸣声
            DEBUG_WARNING(DEBUG_MODULE_AUDIO, "[Callback] 音频播放失败，降级为蜂鸣声");
            speaker.playOneSecondTone();
        }
    }
}

// 初始化函数：设置系统参数和初始化各个模块
void setup() {
    Serial.begin(115200); // 初始化串口通信
    
    // 初始化调试系统（默认 INFO：可看网络轮询等；串口输入 debug off / debug looklook 可调）
    DebugHelper::begin(DEBUG_LEVEL_INFO, DEBUG_MODULE_ALL);

    // 初始化 OLED 显示
    oled.begin();
    oled.showInitializing();
    delay(200);

    /* IO0 按键初始化（与上方 IO0 长按配网配套；恢复配网功能时一并取消注释）
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // 设置为上拉输入模式
    DEBUG_INFO(DEBUG_MODULE_MAIN, "IO0 按键已配置为上拉输入模式");
    */

    // 配置 WiFiManager
    wifiManager.setConfigPortalTimeout(180); // 配网超时 3 分钟
    wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    
    // 关键：关闭自动保存，避免WiFiManager在NTP同步时干扰系统
    wifiManager.setSaveConfigCallback([]() {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "配置已保存");
    });

    // 设置配网回调，显示配网状态
    wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "进入配网模式");
        oled.showWiFiConfig();
    });

    // 尝试自动连接，如果失败则开启 AP 配网
    DEBUG_INFO(DEBUG_MODULE_NETWORK, "尝试连接 WiFi...");
    oled.showWiFiConnecting();

    if (!wifiManager.autoConnect("ESP32-Clock")) {
        DEBUG_WARNING(DEBUG_MODULE_NETWORK, "配网失败，使用编译时间");
        // 配网失败也会继续，使用编译时间
    }

    // WiFi 连接成功
    if (WiFi.status() == WL_CONNECTED) {
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "WiFi 连接成功!");
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "IP 地址：%s", WiFi.localIP().toString().c_str());
        oled.showWiFiConnected(WiFi.localIP().toString().c_str());
        // 减少延迟到500ms，避免看门狗超时
        delay(500);
        
        // 关键：关闭WiFiManager的配置门户，释放资源
        wifiManager.stopConfigPortal();
        DEBUG_INFO(DEBUG_MODULE_NETWORK, "WiFiManager配置门户已关闭");
    }

    // 初始化时钟（含 NTP 同步）
    rtcClock.begin(oled);

    // 初始化 LED
    led.begin();
    led.setInterval(500);
    led.setColor(0, 0, 255);
    led.setBrightness(20);

    // 初始化定时器
    timer.begin();

    // 初始化喇叭
    speaker.begin();

    // 初始化压力传感器
    pressureSensor.begin();
    // 避免上电时 IO4 已为「按下」电平导致误判上升沿、喇叭响一声
    lastPressureState = pressureSensor.isPressureDetected();

    // 读取并显示设备 MAC 地址
    const char* deviceMAC = DeviceMAC::getDeviceMAC();
    DEBUG_INFO(DEBUG_MODULE_MAIN, "Device MAC: %s", deviceMAC);
    oled.showDeviceMAC(deviceMAC);
    delay(300);

    // 初始化远程任务同步
    RemoteTaskSync::init(deviceMAC);
    
    // 初始化闹钟触发器
    AlarmTrigger::init();
    
    // 设置闹钟触发回调
    AlarmTrigger::setAlarmCallback(onAlarmTriggered);
     
    // 初始化网络音频播放器（传入设备 MAC 地址）
    if (audioPlayer.begin(deviceMAC)) {
        DEBUG_INFO(DEBUG_MODULE_AUDIO, "[AudioPlayer] 网络音频播放器初始化成功");
        // 设置默认音量（0-255）
        audioPlayer.setVolume(20);
    } else {
        DEBUG_ERROR(DEBUG_MODULE_AUDIO, "[AudioPlayer] 网络音频播放器初始化失败");
    }
    
    // 从 Flash 读取已存储的闹钟（如果有）
    JsonArray alarms = AlarmTrigger::getAllAlarms();
    if (alarms.size() > 0) {
        DEBUG_INFO(DEBUG_MODULE_ALARM, "[AlarmTrigger] 从 Flash 读取到 %d 个闹钟", alarms.size());
    }

    // 设置闹钟时间（小时，分钟，秒）
    // 这里设置为测试时间，用户可以根据需要修改
    ////////////////////////////////////////////////

    //关闭本地的闹钟
    // timeAlarm.setAlarmTime(14, 8, 0); // 播报时间
    // oled.showAlarmTime(14, 8, 0); // 显示播报文字的时间
    // delay(2000);

    ///////////////////////////////////////////////
    // 无压力时，ADC 值应接近 4100
    // 触发值可自定义，根据实际情况调整
    // 压力检测见 PressureSensor.h（PRESSURE_SIGNAL_ANALOG / 飞线说明）
}

// 主循环：持续更新系统状态
void loop() {
    // 处理串口调试命令（必须放在循环开始，保证及时响应）
    DebugHelper::handleSerialCommand();

    /* IO0 按键检测：长按 2 秒进入重新配网（与文件顶部注释块配套启用）
    currentButtonState = digitalRead(BUTTON_PIN);

    if (currentButtonState == LOW && lastButtonState == HIGH) {
        buttonPressStartTime = millis();
    }

    if (currentButtonState == LOW && millis() - buttonPressStartTime >= BUTTON_PRESS_DURATION) {
        if (!isReconfigMode) {
            DEBUG_INFO(DEBUG_MODULE_MAIN, "IO0 按键长按 2 秒，进入重新配网模式");
            isReconfigMode = true;
            startReconfigWiFi();
        }
    }

    lastButtonState = currentButtonState;
    */

    // 获取压力传感器 ADC 值
    int adcValue = pressureSensor.getFilteredADC();
    
    // 压力传感器检测
    currentPressureState = pressureSensor.isPressureDetected();
    
    // 边沿触发逻辑：当压力从无到有时触发
    if (currentPressureState && !lastPressureState) {
        // 如果正在播放音频，停止播放（用于远程闹钟）
        if (audioPlayer.isPlaying()) {
            DEBUG_INFO(DEBUG_MODULE_SENSOR, "Pressure detected! Stopping audio playback.");
            audioPlayer.stopAudio();
        } else {
            // 非音频播放状态，播放 1 秒蜂鸣声
            DEBUG_INFO(DEBUG_MODULE_SENSOR, "Pressure detected! Playing 1 second tone.");
            speaker.playOneSecondTone();
        }
    }
    
    // 更新压力状态
    lastPressureState = currentPressureState;
    
    // 如果正在播放音频，优先处理音频流，跳过其他所有处理
    if (audioPlayer.isPlaying()) {
        audioPlayer.loop();  // 持续处理音频流
        delay(10);  // 短延时，保证音频流畅
        return;  // 直接返回，不处理其他任务
    }
    
    // 更新 LED 状态
    led.update();

    // 获取当前时间
    DateTime dt = rtcClock.now();
    
    // 获取设备 MAC 地址
    const char* deviceMAC = DeviceMAC::getDeviceMAC();
    
    // 统一刷新屏幕显示（时间、MAC、ADC）
    oled.refreshMainScreen(dt, adcValue, deviceMAC);

    // 检查闹钟是否到达（RTC 闹钟，独立于远程闹钟）
    timeAlarm.checkAlarm(rtcClock.now());

    // 远程任务同步轮询（非阻塞，每 60 秒执行一次）
    RemoteTaskSync::loop();

    // 网络音频播放器循环（处理音频流）
    audioPlayer.loop();

    // 闹钟触发检查（每秒检查，基于 RTC 时间）
    AlarmTrigger::checkAndTriggerAlarm();
    
    delay(5);

    // 统一节拍（默认约 40ms，保证压力 ADC / OLED 跟上快速按压）
    timer.waitForNextCycle();
}
