#include "DeviceMAC.h"
#include <esp_system.h>
#include <stdio.h>
#include <string.h>

static char macString[18];
static bool macInitialized = false;

const char* DeviceMAC::getDeviceMAC() {
    if (!macInitialized) {
        uint8_t mac[6];
        esp_err_t ret = esp_read_mac(mac, ESP_MAC_WIFI_STA);
        
        if (ret == ESP_OK) {
            sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        } else {
            strcpy(macString, "UNKNOWN");
        }
        
        macInitialized = true;
    }
    
    return macString;
}
