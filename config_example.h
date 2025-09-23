#ifndef CONFIG_H
#define CONFIG_H

// Configuration file for IoT Energy Meter
// Edit these values according to your setup

// WiFi Configuration - EDIT THESE VALUES
#ifndef WIFI_SSID
#define WIFI_SSID "your_wifi_ssid"  // Change to your WiFi SSID
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_wifi_password"  // Change to your WiFi password
#endif

// API Configuration - EDIT THESE VALUES
// Production API (commented out)
// #ifndef API_ENDPOINT
// #define API_ENDPOINT "http://your_api_production"
// #endif
// #ifndef API_KEY
// #define API_KEY "your_api_key"
// #endif

// Development API (no security)
#ifndef API_ENDPOINT
#define API_ENDPOINT "your_endpoint"
#endif
#ifndef DEVICE_ID
#define DEVICE_ID "your_name_device"
#endif

// Hardware Configuration
#ifndef PIN_VOLT
#define PIN_VOLT 35
#endif

#ifndef PIN_CURR
#define PIN_CURR 34
#endif

#ifndef SENSITIVITY
#define SENSITIVITY 500.0
#endif

// PIR Sensor Pin (HC-SR501)
#ifndef PIR_PIN
#define PIR_PIN 23
#endif

// DHT22 Sensor Pin
#ifndef DHT22_PIN
#define DHT22_PIN 4
#endif

// Calibration Defaults
#ifndef DEFAULT_VOLT_CAL
#define DEFAULT_VOLT_CAL 1.0
#endif

#ifndef DEFAULT_CURR_CAL
#define DEFAULT_CURR_CAL 111.1
#endif

// Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Timing Configuration
#define SAMPLE_INTERVAL 1000    // Sample every 1 second
#define SEND_INTERVAL 5000      // Send every 5 seconds
#define BUFFER_SIZE 5           // 5 samples to average
#define WIFI_CHECK_INTERVAL 30000  // Check WiFi every 30 seconds

// =========================
// Threshold Configuration
// =========================
// Tegangan (Volt) - batas aman jaringan PLN (sesuaikan)
#ifndef VOLT_MIN
#define VOLT_MIN 180.0
#endif
#ifndef VOLT_MAX
#define VOLT_MAX 250.0
#endif

// Arus (Ampere) - arus maksimum beban (sesuaikan)
#ifndef CURRENT_MAX
#define CURRENT_MAX 25.0
#endif

// Suhu & Kelembapan (DHT22)
#ifndef TEMP_LOW
#define TEMP_LOW 15.0
#endif
#ifndef TEMP_HIGH
#define TEMP_HIGH 35.0
#endif
#ifndef HUM_LOW
#define HUM_LOW 20
#endif
#ifndef HUM_HIGH
#define HUM_HIGH 80
#endif

// PIR active state (HIGH untuk HC-SR501)
#ifndef PIR_ACTIVE_STATE
#define PIR_ACTIVE_STATE HIGH
#endif

// Tambahkan definisi LED indikator PIR
#ifndef LED_PIN
#define LED_PIN 5// GPIO2 biasanya ada LED onboard ESP32
#endif
#ifndef LED_ACTIVE_STATE
#define LED_ACTIVE_STATE HIGH // LED aktif HIGH
#endif

// OLED Configuration
#ifndef OLED_ADDRESS
#define OLED_ADDRESS 0x3C
#endif

// SERIAL & DEBUG Configuration
#ifndef SERIAL_BAUD
#define SERIAL_BAUD 115200
#endif
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED true
#endif

// I2C pins for display
#ifndef SDA_PIN
#define SDA_PIN 21
#endif
#ifndef SCL_PIN
#define SCL_PIN 22
#endif

// TIMING Configuration
#ifndef DISPLAY_UPDATE
#define DISPLAY_UPDATE 1000  // How often to update display (milliseconds)
#endif
#ifndef SAMPLES
#define SAMPLES 100          // Number of samples for sensor reading
#endif

// ZMPT101B (Voltage sensor) settings
#ifndef ZMPT101B_PIN
#define ZMPT101B_PIN 35
#endif
#ifndef VOLTAGE_CALIBRATION
#define VOLTAGE_CALIBRATION 250.0
#endif
#ifndef ZMPT_THRESHOLD
#define ZMPT_THRESHOLD 10
#endif

// SCT013 (Current sensor) settings
#ifndef SCT013_PIN
#define SCT013_PIN 34
#endif
#ifndef CURRENT_CALIBRATION
#define CURRENT_CALIBRATION 30.0
#endif
#ifndef SCT_THRESHOLD
#define SCT_THRESHOLD 5
#endif

// System constants
#ifndef ADC_REF_VOLTAGE
#define ADC_REF_VOLTAGE 3.3
#endif
#ifndef ADC_RESOLUTION
#define ADC_RESOLUTION 4095.0
#endif
#ifndef DC_OFFSET
#define DC_OFFSET 1.65
#endif

#endif //