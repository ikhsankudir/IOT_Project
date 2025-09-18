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
#ifndef API_ENDPOINT
#define API_ENDPOINT "your_end_point"  // Change to your API endpoint
#endif

#ifndef DEVICE_ID
#define DEVICE_ID "your_device_id"  // Change to your device ID
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

#endif // CONFIG_H