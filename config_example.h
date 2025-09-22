//=============================================================================
// ESP32 Energy Monitor - Configuration File
//=============================================================================

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_pass"

// Server Configuration
#define SERVER_URL "http://your_endpoint"
#define DEVICE_ID "ESP32_001"

//=============================================================================
// PIN CONFIGURATION - STEP 1: ADD NEW SENSOR PINS HERE
//=============================================================================
// Current sensors
#define ZMPT101B_PIN A0     // Voltage sensor
#define SCT013_PIN A3       // Current sensor

// I2C pins for display
#define SDA_PIN 21
#define SCL_PIN 22

// ADD NEW SENSOR PINS BELOW:
// Example: #define DHT22_PIN 4
// Example: #define RELAY_PIN 5
// Example: #define LED_PIN 2

//=============================================================================
// OLED Configuration
#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//=============================================================================
// SERIAL & DEBUG Configuration
#define SERIAL_BAUD 115200
#define DEBUG_ENABLED true    // Set to false to disable serial output

//=============================================================================
// TIMING Configuration
#define SEND_INTERVAL 5000   // How often to send data (milliseconds)
#define DISPLAY_UPDATE 1000  // How often to update display (milliseconds)
#define SAMPLES 100          // Number of samples for sensor reading

//=============================================================================
// SENSOR CALIBRATION & THRESHOLDS - STEP 2: ADD NEW SENSOR CONSTANTS HERE
//=============================================================================
// ZMPT101B (Voltage sensor) settings
#define VOLTAGE_CALIBRATION 250.0  // Adjust based on your setup
#define ZMPT_THRESHOLD 10          // Minimum variation to detect AC signal

// SCT013 (Current sensor) settings
#define CURRENT_CALIBRATION 30.0   // Adjust based on your setup
#define SCT_THRESHOLD 5            // Minimum variation to detect AC signal

// System constants
#define ADC_REF_VOLTAGE 3.3
#define ADC_RESOLUTION 4095.0
#define DC_OFFSET 1.65             // DC offset for AC sensors

// ADD NEW SENSOR CALIBRATIONS BELOW:
// Example: #define DHT22_TEMP_OFFSET 2.5
// Example: #define RELAY_ON_THRESHOLD 3.0
// Example: #define LED_BRIGHTNESS 255

#endif
