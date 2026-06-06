#include "Clock.h"
#include "DebugHelper.h"
#include <Arduino.h>
#include <time.h>
#include <esp_task_wdt.h>

// NTP服务器配置
const char* ntpServer1 = "ntp.aliyun.com";  // 阿里云NTP服务器
const char* ntpServer2 = "cn.pool.ntp.org";  // 中国NTP服务器池
const long gmtOffset_sec = 8 * 3600;      // 东八区（UTC+8）
const int daylightOffset_sec = 0;          // 无夏令时


Clock::Clock() : initialized(false), ntpSynced(false) {}  //初始化时钟对象，设置初始状态为未初始化

/**
 * 初始化时钟实现
 * 1. 检查WiFi连接状态
 * 2. 如果WiFi已连接，尝试从NTP服务器同步时间
 * 3. 如果NTP同步失败或WiFi未连接，使用编译时间作为回退
 * @param display OLED显示对象引用，用于显示同步状态
 */
void Clock::begin(OLED_Display &display) {
    // 如果已经初始化，直接返回
    if (initialized) return;

    // 喂看门狗，防止同步过程中超时
    esp_task_wdt_reset();
    DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "[Clock] 开始初始化时钟...");

    // 检查WiFi是否已连接
    if (WiFi.status() == WL_CONNECTED) {
        DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "[Clock] WiFi已连接，尝试NTP同步...");
        // 尝试从NTP同步时间
        if (setTimeFromNTP(display)) {
            ntpSynced = true;
            initialized = true;
            esp_task_wdt_reset();  // 喂看门狗
            DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "[Clock] 时钟初始化完成（使用NTP时间）");
            return;
        }
        DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "[Clock] NTP同步失败，回退到编译时间");
    } else {
        DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "[Clock] WiFi未连接，使用编译时间");
    }

    // 使用编译时间作为回退
    DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "[Clock] 使用编译时间");
    setTimeFromCompileTime();
    display.showCompileTimeFallback();
    // 减少延迟到500ms，避免看门狗超时
    delay(500);
    initialized = true;
    
    // 喂看门狗
    esp_task_wdt_reset();
    DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "[Clock] 时钟初始化完成（使用编译时间）");
}

DateTime Clock::now() {    //获取当前时间实现,调用RTC对象的now()方法获取当前时间
    return rtc.now();
}

/**
 * 从编译时间设置系统时间实现
 * 1. 解析编译日期和时间
 * 2. 创建DateTime对象
 * 3. 初始化RTC
 * 4. 打印设置结果
 */
void Clock::setTimeFromCompileTime() {
    DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "RTC使用编译时间...");

    int year, month, day, hour, minute, second;
    // 解析编译日期
    parseCompileDate(year, month, day);
    // 解析编译时间
    parseCompileTime(hour, minute, second);

    // 创建编译时间对象
    DateTime compileTime(year, month, day, hour, minute, second);
    // 初始化RTC
    rtc.begin(compileTime);

    // 打印设置的时间
    DEBUG_PRINTF(DEBUG_MODULE_MAIN, "RTC时间已设置(编译时间): %04d-%02d-%02d %02d:%02d:%02d\n",
                  year, month, day, hour, minute, second);
}

/**
 * 从NTP服务器同步时间实现
 * 1. 等待2秒让WiFi网络栈稳定
 * 2. 显示同步状态
 * 3. 配置NTP服务器
 * 4. 等待NTP同步，最多重试30次（每次1秒）
 * 5. 如果同步成功，设置RTC并返回true
 * 6. 如果同步失败，显示失败信息并返回false
 * @param display OLED显示对象引用，用于显示同步状态
 * @return 同步是否成功
 */
bool Clock::setTimeFromNTP(OLED_Display &display) {
    // 等待网络栈稳定（关键！WiFiManager刚完成后不能立即配置NTP）
    DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "等待网络栈稳定...");
    delay(2000);
    esp_task_wdt_reset();  // 喂看门狗

    DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "开始配置NTP...");
    // 显示同步状态
    display.showNTPSyncing();

    // 配置NTP服务器和时区
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    
    // 等待配置生效
    delay(500);
    esp_task_wdt_reset();  // 喂看门狗

    // 等待NTP同步，最多重试30次（30秒）
    struct tm timeinfo;
    int retry = 0;
    const int maxRetries = 30;

    DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "开始等待NTP同步...");

    while (!getLocalTime(&timeinfo, 1000) && retry < maxRetries) {
        // 每1秒重试一次，使用非阻塞等待
        retry++;
        
        // 每5次打印一次，减少串口输出
        if (retry % 5 == 0) {
            DEBUG_PRINTF(DEBUG_MODULE_MAIN, "NTP同步中... %d秒\n", retry);
            // 喂看门狗，防止超时复位
            esp_task_wdt_reset();
        }
    }

    // 检查是否同步成功
    if (retry >= maxRetries) {
        DEBUG_INFO(DEBUG_MODULE_MAIN, %s, "NTP同步失败！");
        display.showNTPFailed();
        // 减少延迟到500ms，避免看门狗超时
        delay(500);
        return false;
    }

    // NTP同步成功，设置RTC
    DateTime ntpTime(
        timeinfo.tm_year + 1900,  // tm_year是从1900开始的年数
        timeinfo.tm_mon + 1,      // tm_mon是从0开始的月份
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );
    rtc.begin(ntpTime);

    // 打印同步成功的时间
    DEBUG_PRINTF(DEBUG_MODULE_MAIN, "NTP同步成功: %04d-%02d-%02d %02d:%02d:%02d\n",
                  timeinfo.tm_year + 1900,
                  timeinfo.tm_mon + 1,
                  timeinfo.tm_mday,
                  timeinfo.tm_hour,
                  timeinfo.tm_min,
                  timeinfo.tm_sec);

    // 显示同步成功信息
    display.showNTPSuccess();
    // 减少延迟到500ms，避免看门狗超时
    delay(500);
    return true;
}

/**
 * 解析编译日期实现
 * 从__DATE__宏中解析年、月、日
 * @param year 输出参数，存储年份
 * @param month 输出参数，存储月份
 * @param day 输出参数，存储日期
 */
void Clock::parseCompileDate(int &year, int &month, int &day) {
    const char *dateStr = __DATE__;  // __DATE__宏格式: "Jan  2 2024"
    char monthStr[4];
    // 解析日期字符串
    sscanf(dateStr, "%3s %d %d", monthStr, &day, &year);

    // 将月份字符串转换为数字
    if (strcmp(monthStr, "Jan") == 0) month = 1;
    else if (strcmp(monthStr, "Feb") == 0) month = 2;
    else if (strcmp(monthStr, "Mar") == 0) month = 3;
    else if (strcmp(monthStr, "Apr") == 0) month = 4;
    else if (strcmp(monthStr, "May") == 0) month = 5;
    else if (strcmp(monthStr, "Jun") == 0) month = 6;
    else if (strcmp(monthStr, "Jul") == 0) month = 7;
    else if (strcmp(monthStr, "Aug") == 0) month = 8;
    else if (strcmp(monthStr, "Sep") == 0) month = 9;
    else if (strcmp(monthStr, "Oct") == 0) month = 10;
    else if (strcmp(monthStr, "Nov") == 0) month = 11;
    else if (strcmp(monthStr, "Dec") == 0) month = 12;
    else month = 1;  // 默认为1月
}

/**
 * 解析编译时间实现
 * 从__TIME__宏中解析时、分、秒
 * @param hour 输出参数，存储小时
 * @param minute 输出参数，存储分钟
 * @param second 输出参数，存储秒
 */
void Clock::parseCompileTime(int &hour, int &minute, int &second) {
    const char *timeStr = __TIME__;  // __TIME__宏格式: "12:34:56"
    // 解析时间字符串
    sscanf(timeStr, "%d:%d:%d", &hour, &minute, &second);
}
