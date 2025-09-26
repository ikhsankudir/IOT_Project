# ESP32 Multi-Sensor IoT Monitor

ESP32-based IoT monitoring system with electrical energy, motion, and environmental sensors. Sends structured data to HTTP API with comprehensive JSON format.

## 📋 Project Description

This project is an IoT monitoring system with features:
- **Multi-sensor monitoring**: Electrical energy, motion, temperature/humidity
- **Real-time data transmission**: Sends data every 5 seconds to HTTP API
- **OLED display**: Real-time display of all sensors
- **Dual API support**: Production and Development environments
- **FreeRTOS multitasking**: Performance optimization with dual-core ESP32
- **Auto-reconnect**: WiFi and system health monitoring
- **Structured JSON payload**: Structured and complete data format

## 🔧 Hardware Components

### Sensor Array
- **PZEM-004T**: AC power meter with voltage, current, power, energy (RX:16, TX:17) - Comprehensive electrical monitoring
- **HC-SR501 PIR**: Motion sensor (Pin 23) - Presence detection
- **DHT22**: Temperature & humidity sensor (Pin 4) - Environmental monitoring
- **IR Proximity Sensor**: Obstacle detection (Pin 25) - Proximity monitoring
- **RCWL-0516**: Microwave radar motion sensor (Pin 26) - Advanced motion detection
- **ESP32**: Main microcontroller with dual-core
- **OLED SSD1306**: 128x64 I2C display (SDA:21, SCL:22)

### Technical Specifications
- **Microcontroller**: ESP32 dual-core 240MHz
- **ADC Resolution**: 12-bit (0-4095)
- **Sampling Rate**: 100 samples per sensor per reading
- **Data Transmission**: Every 5 seconds
- **WiFi**: Auto-reconnect with RSSI monitoring
- **Memory**: ~300KB free heap, SPIFFS storage

## 📡 API Configuration

### Development API (Current)
```c
#define API_ENDPOINT "your_endpoint"
// No authentication required
```

### Production API (Commented)
```c
// #define API_ENDPOINT "your_endpoint"
// #define API_KEY "your_production_api_key"
```

### JSON Payload Format
```json
{
  "version": "1.2",
  "ts": "2025-09-22T14:20:15Z",
  "seq": 141463,
  "tenant": "hospital-abc",
  "device": {
    "id": "esp32-5d671568",
    "type": "esp32",
    "fw": "2.1.0",
    "name": "IoT Multi-Board A",
    "location": {
      "room": "ICU-01",
      "lat": -6.2,
      "lng": 106.8,
      "alt_m": 45
    },
    "tags": ["demo", "multisensor", "realistic-sim"]
  },
  "network": {
    "conn": "wifi",
    "ip": "10.10.10.10",
    "rssi_dbm": -57,
    "snr_db": null,
    "mac": "24:6F:28:65:BB:CC"
  },
  "power": {
    "battery_pct": null,
    "voltage_v": 5.0,
    "charging": true
  },
  "resources": {
    "uptime_s": 187320,
    "cpu_pct": 14.2,
    "mem_pct": 49.8,
    "fs_used_pct": 68.5,
    "heap_free_kb": 176,
    "flash_free_kb": 980,
    "temp_c": 41.8
  },
  "agg": {
    "window_s": 5,
    "method": "raw"
  },
  "data": [
    {
      "sensor": "pzem-004t",
      "category": "power",
      "iface": "serial",
      "unit_system": "SI",
      "observations": {
        "voltage_v": 212.0,
        "current_a": 0.11,
        "power_w": 14.4,
        "energy_kwh": 0.065,
        "frequency_hz": 50.0,
        "power_factor": 0.95
      },
      "quality": {"status": "ok", "calibrated": true, "errors": [], "notes": "PZEM-004T power meter dengan split CT untuk monitoring listrik komprehensif."}
    },
    {
      "sensor": "hc-sr501",
      "category": "motion",
      "iface": "digital",
      "unit_system": "SI",
      "observations": {"motion_detected": false},
      "quality": {"status": "ok", "calibrated": true, "errors": [], "notes": "Sensor gerak PIR HC-SR501 untuk deteksi kehadiran."}
    },
    {
      "sensor": "dht22",
      "category": "env",
      "iface": "digital",
      "unit_system": "SI",
      "observations": {"temperature_c": 24.2, "humidity_pct": 51.6},
      "quality": {"status": "ok", "calibrated": true, "errors": [], "notes": "Sensor DHT22 untuk monitoring suhu dan kelembapan ruangan."}
    },
    {
      "sensor": "ir-proximity",
      "category": "proximity",
      "iface": "digital",
      "unit_system": "SI",
      "observations": {"obstacle_detected": false},
      "quality": {"status": "ok", "calibrated": true, "errors": [], "notes": "Sensor IR proximity untuk deteksi obstacle/hambatan."}
    },
    {
      "sensor": "rcwl-0516",
      "category": "motion",
      "iface": "digital",
      "unit_system": "SI",
      "observations": {"motion_detected": false},
      "quality": {"status": "ok", "calibrated": true, "errors": [], "notes": "Sensor radar gelombang mikro RCWL-0516 untuk deteksi gerakan."}
    }
  ]
}
```

## 🚀 Installation & Setup

### 1. Hardware Preparation
1. Connect all sensors according to pin configuration
2. Ensure OLED display is connected to I2C (SDA:21, SCL:22)
3. Provide 5V power supply to ESP32

### 2. Software Setup
1. **Install Arduino IDE** with ESP32 board support
2. **Install required libraries:**
   - WiFi
   - HTTPClient
   - ArduinoJson
   - DHT sensor library
   - Adafruit GFX
   - Adafruit SSD1306

3. **WiFi Configuration:**
   ```c
   #define WIFI_SSID "your_wifi_name"
   #define WIFI_PASSWORD "your_wifi_password"
   ```

4. **Upload code** to ESP32

### 3. Sensor Calibration
- **PZEM-004T**: No calibration needed - factory calibrated
- **Thresholds**: Adjust safety limits in config.h

## 📊 Monitoring & Display

### OLED Display Layout
```
ESP32 Monitor          W H
----------------
V:212.0V I:0.11A
T:24.2C H:52%
PIR:NO IR:NO
PWR:OK STATUS:NORMAL
```

### Serial Debug Output
```
==== STATUS ENERGI ====
V: 212.0V  I: 0.11A  P: 14.4W  E: 0.065kWh
PIR: IDLE
T: 24.2C  H: 52%
WiFi: connected  RSSI: -47
Uptime: 1339s  RAM: 188KB
```

## 🔄 FreeRTOS Task Architecture

### Core 0 (Network Task)
- WiFi management & reconnect
- HTTP data transmission
- API communication

### Core 1 (Application Tasks)
- Sensor data acquisition
- OLED display updates
- Real-time PIR monitoring

### Task Priorities
- Network Task: Priority 2 (High)
- Sensor Task: Priority 2 (High)
- Display Task: Priority 1 (Normal)

## ⚙️ Advanced Configuration

### Threshold Configuration
```c
// Voltage limits (Volts)
#define VOLT_MIN 180.0
#define VOLT_MAX 250.0

// Current limits (Amps)
#define CURRENT_MAX 25.0

// Temperature & Humidity
#define TEMP_LOW 15.0
#define TEMP_HIGH 35.0
#define HUM_LOW 20
#define HUM_HIGH 80
```

### Timing Configuration
```c
#define SAMPLE_INTERVAL 1000    // Sensor sampling (ms)
#define SEND_INTERVAL 5000      // Data transmission (ms)
#define DISPLAY_UPDATE 1000     // Display refresh (ms)
#define WIFI_CHECK_INTERVAL 30000 // WiFi health check (ms)
```

## 🔧 Troubleshooting

### Common Issues
1. **WiFi Connection Failed**
   - Check SSID and password
   - Verify WiFi signal strength
   - Check firewall settings

2. **Sensor Reading Errors**
   - Verify sensor connections
   - Check power supply (5V)
   - Calibrate sensor values

3. **HTTP Transmission Failed**
   - Check API endpoint URL
   - Verify network connectivity
   - Check API server status

4. **OLED Display Issues**
   - Verify I2C connections (SDA:21, SCL:22)
   - Check OLED power supply
   - Verify I2C address (0x3C)

### Debug Mode
Enable debug output in `config.h`:
```c
#define DEBUG_ENABLED true
#define SERIAL_BAUD 115200
```

## 📈 Performance Metrics

- **CPU Usage**: ~15% (dual-core optimized)
- **Memory Usage**: ~50% heap utilization
- **Network Latency**: <100ms per transmission
- **Power Consumption**: ~0.5W standby, 1.2W active

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ESP32 community for excellent documentation
- Arduino framework developers
- Open source sensor libraries contributors
