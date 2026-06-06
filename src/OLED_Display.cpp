#include "OLED_Display.h"

OLED_Display::OLED_Display()
    : u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE) {}

void OLED_Display::begin() {
    u8g2.begin();
}

void OLED_Display::showInitializing() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "Initializing...");
    u8g2.sendBuffer();
}

void OLED_Display::showTime(const DateTime &dt) {
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(20, 45, timeStr);
    u8g2.sendBuffer();
}

// WiFi状态显示
void OLED_Display::showWiFiConfig() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 15, "WiFi Config...");
    u8g2.drawStr(0, 30, "AP: ESP32-Clock");
    u8g2.drawStr(0, 45, "192.168.4.1");
    u8g2.sendBuffer();
}

void OLED_Display::showWiFiConnecting() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 30, "WiFi Connecting...");
    u8g2.sendBuffer();
}

void OLED_Display::showWiFiConnected(const char* ip) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "WiFi Connected");
    char ipStr[32];
    snprintf(ipStr, sizeof(ipStr), "IP: %s", ip);
    u8g2.drawStr(0, 40, ipStr);
    u8g2.sendBuffer();
}

// NTP状态显示
void OLED_Display::showNTPSyncing() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 30, "Syncing Time...");
    u8g2.sendBuffer();
}

void OLED_Display::showNTPSuccess() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 30, "Time Synced!");
    u8g2.sendBuffer();
}

void OLED_Display::showNTPFailed() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "NTP Failed");
    u8g2.sendBuffer();
}

void OLED_Display::showCompileTimeFallback() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "Using Compile");
    u8g2.drawStr(0, 35, "Time");
    u8g2.sendBuffer();
}

// 提醒状态显示
void OLED_Display::showAlarmTime(uint8_t hour, uint8_t minute, uint8_t second) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "Alarm:");
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
    u8g2.drawStr(30, 35, timeStr);
    u8g2.drawStr(0, 50, "Ready to trigger");
    u8g2.sendBuffer();
}

void OLED_Display::showAlarmTriggered() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(10, 45, "Alarm!");
    u8g2.sendBuffer();
    delay(2000);  // 显示2秒
}

void OLED_Display::showPressureADC(int adcValue) {
    char adcStr[10];
    sprintf(adcStr, "ADC: %d", adcValue);
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 60, adcStr);
    u8g2.sendBuffer();
}

void OLED_Display::showDeviceMAC(const char* mac) {
    char macLabel[25];
    snprintf(macLabel, sizeof(macLabel), "MAC: %s", mac);
    
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 50, macLabel);
}

void OLED_Display::refreshMainScreen(const DateTime &dt, int adcValue, const char* mac) {
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
    
    char adcStr[10];
    sprintf(adcStr, "ADC: %d", adcValue);
    
    char macLabel[25];
    snprintf(macLabel, sizeof(macLabel), "MAC: %s", mac);
    
    u8g2.clearBuffer();
    
    // 显示 ADC 值（顶部，小字）
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, adcStr);
    
    // 显示时间（中间，大字）
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(20, 45, timeStr);
    
    // 显示 MAC 地址（底部，小字）
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 60, macLabel);
    
    u8g2.sendBuffer();
}
