## 1. Create DeviceMAC Module

- [x] 1.1 Create src/DeviceMAC.h header file with function declarations
- [x] 1.2 Create src/DeviceMAC.cpp implementation file with esp_read_mac call
- [x] 1.3 Implement static global variable for MAC caching
- [x] 1.4 Implement MAC address to hex string conversion

## 2. Update OLED Display

- [x] 2.1 Add showDeviceMAC() method to OLED_Display.h
- [x] 2.2 Implement showDeviceMAC() in OLED_Display.cpp to display at row 50
- [x] 2.3 Ensure no overlap with existing display content

## 3. Integrate into Main Application

- [x] 3.1 Include DeviceMAC.h in main.cpp
- [x] 3.2 Call getDeviceMAC() in setup() and print to Serial
- [x] 3.3 Call oled.showDeviceMAC() to display on screen

## 4. Verify Implementation

- [x] 4.1 Build project and check for compilation errors
- [ ] 4.2 Deploy to device and verify MAC displays correctly
- [ ] 4.3 Check Serial output shows correct MAC address
- [ ] 4.4 Verify no overlap with other screen content
