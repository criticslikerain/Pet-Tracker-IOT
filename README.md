# Pet GPS Tracker with ESP32
# (USE MASTER BRANCH) 

## Overview
A GPS tracking system for pets using ESP32, featuring dual connectivity (GPRS/WiFi) and real-time location monitoring through Telegram and Firebase. This project enables pet owners to request and receive their pet's location on demand.

## Materials Needed

### Core Components
1. ESP32 Development Board
2. NEO-6M GPS Module
3. SIM800L GSM/GPRS Module
4. SIM Card with data plan (Globe recommended for Philippines)
5. 3.7V Li-ion Battery
6. TP4056 Battery Charging Module
7. Step-up Boost Converter Module (3.7V to 5V)
8. PCB Board or Breadboard
9. Project Box/Case (waterproof recommended)

### Additional Components
- Voltage Regulator LM1117 3.3V
- Capacitors:
  - 100μF electrolytic
  - 10μF electrolytic
  - 100nF ceramic
- Resistors:
  - 2x 10kΩ
  - 2x 1kΩ
- LED indicators
- Toggle switch
- Connecting wires
- Male/Female pin headers

## Wiring Connections

### GPS Module (NEO-6M) to ESP32
```
NEO-6M      ESP32
VCC    -->  3.3V
GND    -->  GND
TX     -->  GPIO26 (RX)
RX     -->  GPIO27 (TX)
```

### SIM800L to ESP32
```
SIM800L     ESP32
VCC    -->  5V (from boost converter)
GND    -->  GND
TX     -->  GPIO16 (RX)
RX     -->  GPIO17 (TX)
```

### Power Supply Configuration
```
Battery --> TP4056 --> Boost Converter --> SIM800L
       |
       --> 3.3V Regulator --> ESP32 & GPS Module
```

## Features
- 📍 Real-time GPS tracking
- 📱 Dual connectivity (GPRS via SIM800L & WiFi fallback)
- 🤖 Telegram bot integration for location requests
- 🔥 Firebase Realtime Database integration
- 🔄 Automatic failover between GPRS and WiFi
- 📊 Location data logging
- 🔋 Battery monitoring
- ⚡ Low power consumption design

## Power Considerations
- SIM800L peak current: up to 2A during transmission
- Operating voltage requirements:
  - ESP32: 3.3V
  - NEO-6M: 3.3V
  - SIM800L: 3.4V-4.4V
- Battery life: ~24 hours with 3000mAh battery (typical usage)

## Assembly Instructions

1. **Power Supply Setup**
   - Connect battery to TP4056 module
   - Connect TP4056 output to boost converter
   - Connect 3.3V regulator for ESP32 and GPS

2. **ESP32 Connections**
   - Connect all grounds together
   - Wire GPS and SIM800L according to pinout diagram
   - Add decoupling capacitors near power inputs

3. **Antenna Considerations**
   - Keep GPS antenna facing skyward
   - Maintain distance between GPS and GSM antennas
   - Avoid metal enclosures blocking signals

4. **Final Assembly**
   - Mount components securely in case
   - Ensure proper ventilation
   - Waterproof all connections if used outdoors

## Software Setup
[Previous software setup instructions remain the same...]

## Troubleshooting Guide

### Common Issues
1. **No GPS Signal**
   - Check antenna connection
   - Ensure clear view of sky
   - Verify power supply voltage

2. **GPRS Connection Fails**
   - Check SIM card balance
   - Verify APN settings
   - Check signal strength

3. **Battery Issues**
   - Verify charging circuit
   - Check battery voltage
   - Inspect power connections

## Performance Specifications
- GPS Accuracy: ~2.5 meters
- GPRS Response Time: 2-5 seconds
- WiFi Response Time: 1-2 seconds
- Operating Temperature: -20°C to 60°C
- Battery Life: 18-24 hours (typical use)

[Rest of the previous README content remains the same...]
