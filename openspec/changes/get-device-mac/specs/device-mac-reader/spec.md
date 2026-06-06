## ADDED Requirements

### Requirement: Device MAC Address Reader
The system SHALL read the 6-byte Base MAC address from ESP32-S3 eFuse using the `esp_read_mac` function.

#### Scenario: Successful MAC read
- **WHEN** the device starts up and `getDeviceMAC()` is called
- **THEN** the system SHALL read 6 bytes from eFuse and return a 17-character uppercase hexadecimal string with colons (e.g., "24:0A:C4:A3:8B:FF")

### Requirement: MAC Address Caching
The system SHALL cache the MAC address in a static global variable to avoid repeated hardware reads.

#### Scenario: First call reads hardware
- **WHEN** `getDeviceMAC()` is called for the first time after boot
- **THEN** the system SHALL call `esp_read_mac()` to read from eFuse and cache the result

#### Scenario: Subsequent calls return cached value
- **WHEN** `getDeviceMAC()` is called after the first call
- **THEN** the system SHALL return the cached MAC string without re-reading hardware

### Requirement: MAC Address Format
The system SHALL convert the MAC address to a specific string format with colons.

#### Scenario: Format conversion
- **WHEN** the 6-byte MAC is read from eFuse
- **THEN** the system SHALL convert each byte to 2 uppercase hexadecimal characters and insert colons between them (e.g., "24:0A:C4:A3:8B:FF")

#### Scenario: Byte to hex conversion
- **WHEN** the MAC bytes are [0x24, 0x0A, 0xC4, 0xA3, 0x8B, 0xFF]
- **THEN** the system SHALL output "24:0A:C4:A3:8B:FF"
