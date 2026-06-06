## ADDED Requirements

### Requirement: MAC Display on OLED
The system SHALL display the MAC address on the OLED screen without overlapping existing content.

#### Scenario: Display MAC on screen
- **WHEN** `showDeviceMAC()` is called
- **THEN** the system SHALL display the MAC address at row 50 on the OLED screen

#### Scenario: No overlap with other content
- **WHEN** the screen shows time (row 45), MAC (row 50), and ADC value (row 60)
- **THEN** all content SHALL be visible without overlap

### Requirement: MAC Print to Serial
The system SHALL print the MAC address to the serial port for debugging purposes.

#### Scenario: Print MAC to serial
- **WHEN** `getDeviceMAC()` is called during initialization
- **THEN** the system SHALL print "Device MAC: <MAC>" to Serial at 115200 baud
