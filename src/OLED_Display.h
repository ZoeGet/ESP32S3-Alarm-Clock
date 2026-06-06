#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <U8g2lib.h>
#include <RTClib.h>

#define SDA_PIN 8
#define SCL_PIN 9

// OLED显示类：控制0.96寸OLED屏幕显示
class OLED_Display {
public:
    // 构造函数：初始化OLED显示对象
    OLED_Display();
    
    // 初始化OLED显示
    void begin();
    
    // 显示初始化信息
    void showInitializing();
    
    // 显示当前时间
    void showTime(const DateTime &dt);
    
    // WiFi状态显示
    void showWiFiConfig();       // 显示配网模式信息
    void showWiFiConnecting();    // 显示连接中状态
    void showWiFiConnected(const char* ip);  // 显示连接成功信息
    
    // NTP状态显示
    void showNTPSyncing();        // 显示NTP同步中
    void showNTPSuccess();        // 显示NTP同步成功
    void showNTPFailed();         // 显示NTP同步失败
    void showCompileTimeFallback();  // 显示使用编译时间
    
    // 提醒状态显示
    void showAlarmTime(uint8_t hour, uint8_t minute, uint8_t second);  // 显示闹钟时间
    void showAlarmTriggered();     // 显示闹钟触发信息
    
    // 显示压力传感器ADC值
    void showPressureADC(int adcValue);
    
    void showDeviceMAC(const char* mac);
    
    // 刷新主屏幕显示（时间、MAC、ADC）
    void refreshMainScreen(const DateTime &dt, int adcValue, const char* mac);
    
private:
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2;  // U8g2库对象
};

#endif // OLED_DISPLAY_H
