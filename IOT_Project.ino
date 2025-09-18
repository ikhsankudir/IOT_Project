/*
  ZMPT101B + SCT013 HTTP API Publisher - Clean & Efficient
  - Sample sensors every 1 second
  - Send averaged data every 5 seconds to HTTP API
  - Minimal code, maximum stability
  - Auto-reconnect WiFi
  - Configuration via config.h (safe for GitHub)
  
  Data sent: JSON with averaged sensor readings
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <ZMPT101B.h>
#include <Adafruit_SSD1306.h>
#include <EmonLib.h>
#include <Preferences.h>
#include "config.h"

// --- WiFi & API Config (from config.h) ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* api_endpoint = API_ENDPOINT;
const char* device_id = DEVICE_ID;

// --- OLED ---
Adafruit_SSD1306 disp(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Sensors ---
ZMPT101B voltSensor(PIN_VOLT, 50.0);
EnergyMonitor emon;

// --- Persistent Calibration ---
Preferences prefs;
float calVolt = DEFAULT_VOLT_CAL;
float calCurr = DEFAULT_CURR_CAL;

// --- Timing ---
unsigned long lastSampleTime = 0;
unsigned long lastSendTime = 0;
unsigned long bootTime = 0;
unsigned long lastWifiCheck = 0;

// --- Data Buffers for Averaging ---
float voltageBuffer[BUFFER_SIZE];
float currentBuffer[BUFFER_SIZE];
int bufferIndex = 0;
int samplesCollected = 0;
bool bufferFull = false;

// --- Current Status ---
float avgVoltageRaw = 0.0;
float avgCurrentRaw = 0.0;
int wifiRssi = 0;
String systemStatus = "Starting";
int httpResponseCode = 0;
unsigned long totalSamplesSent = 0;
bool verboseMode = false;  // Control serial output verbosity

// --- ESP Resources & Sensor Status ---
struct ESPResources {
  uint32_t freeHeap;
  uint32_t totalHeap;
  float cpuTemp;
  uint8_t cpuFreq;
  uint32_t flashSize;
  uint32_t freeFlash;
} espRes;

struct SensorStatus {
  bool voltageOK;
  bool currentOK;
  bool oledOK;
  float voltageNoise;
  float currentNoise;
  unsigned long lastGoodReading;
} sensorStat;

// --- WiFi Connection ---
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.printf("WiFi connected: %s\n", WiFi.localIP().toString().c_str());
    wifiRssi = WiFi.RSSI();
    systemStatus = "WiFi OK";
  } else {
    Serial.println("\\nWiFi connection failed!");
    systemStatus = "WiFi Failed";
  }
}

// --- Check ESP Resources ---
void updateESPResources() {
  espRes.freeHeap = ESP.getFreeHeap();
  espRes.totalHeap = ESP.getHeapSize();
  espRes.cpuTemp = temperatureRead();
  espRes.cpuFreq = ESP.getCpuFreqMHz();
  espRes.flashSize = ESP.getFlashChipSize();
  espRes.freeFlash = ESP.getFreeSketchSpace();
}

// --- Check Sensor Health ---
void checkSensorHealth(float voltage, float current) {
  // Voltage sensor check
  sensorStat.voltageOK = (voltage > 50 && voltage < 300);
  if (!sensorStat.voltageOK) {
    sensorStat.voltageNoise = abs(voltage - avgVoltageRaw);
  }
  
  // Current sensor check  
  sensorStat.currentOK = (current >= 0 && current < 50);
  if (sensorStat.currentOK) {
    sensorStat.currentNoise = abs(current - avgCurrentRaw);
    sensorStat.lastGoodReading = millis();
  }
  
  // OLED check (simple)
  sensorStat.oledOK = true; // Will be false if display fails
}

// --- Sample Sensors ---
void sampleSensors() {
  // Sample voltage with mini-averaging for stability
  float vSum = 0;
  for (int i = 0; i < 3; i++) {
    vSum += voltSensor.getRmsVoltage();
    delay(10);
  }
  float voltageRaw = vSum / 3.0;
  
  // Sample current
  float currentRaw = emon.calcIrms(800); // Reduced samples for faster response
  if (currentRaw < 0.005) currentRaw = 0.0; // Noise threshold
  
  // Check sensor health
  checkSensorHealth(voltageRaw, currentRaw);
  
  // Store in circular buffer
  voltageBuffer[bufferIndex] = voltageRaw;
  currentBuffer[bufferIndex] = currentRaw;
  
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  
  if (samplesCollected < BUFFER_SIZE) {
    samplesCollected++;
  } else {
    bufferFull = true;
  }
  
  // Simplified output - only show when needed
  if (verboseMode || !sensorStat.voltageOK || !sensorStat.currentOK) {
    Serial.printf("V=%.1f I=%.3f [%s%s]\\n", 
                  voltageRaw, currentRaw,
                  sensorStat.voltageOK ? "✓" : "✗V",
                  sensorStat.currentOK ? "✓" : "✗I");
  }
}

// --- Calculate Averages ---
void calculateAverages() {
  if (samplesCollected == 0) return;
  
  float vSum = 0, iSum = 0;
  int samples = bufferFull ? BUFFER_SIZE : samplesCollected;
  
  for (int i = 0; i < samples; i++) {
    vSum += voltageBuffer[i];
    iSum += currentBuffer[i];
  }
  
  avgVoltageRaw = vSum / samples;
  avgCurrentRaw = iSum / samples;
}

// --- Send Data to API ---
void sendToAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    systemStatus = "WiFi Lost";
    return;
  }
  
  calculateAverages();
  
  HTTPClient http;
  http.begin(api_endpoint);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000); // 10 second timeout
  
  // Create JSON payload
  StaticJsonDocument<300> doc;
  
  doc["device_id"] = device_id;
  doc["timestamp"] = millis() - bootTime; // Uptime in ms
  doc["samples_count"] = bufferFull ? BUFFER_SIZE : samplesCollected;
  
  // Raw sensor data (averaged)
  doc["voltage_raw"] = round(avgVoltageRaw * 1000) / 1000.0; // 3 decimal places
  doc["current_raw"] = round(avgCurrentRaw * 10000) / 10000.0; // 4 decimal places
  
  // Calibration factors
  doc["voltage_cal"] = calVolt;
  doc["current_cal"] = calCurr;
  
  // System info
  doc["wifi_rssi"] = wifiRssi;
  doc["uptime_seconds"] = (millis() - bootTime) / 1000;
  doc["total_samples_sent"] = totalSamplesSent;
  
  // ESP Resources
  updateESPResources();
  JsonObject esp = doc.createNestedObject("esp_resources");
  esp["free_heap"] = espRes.freeHeap;
  esp["heap_usage_pct"] = ((espRes.totalHeap - espRes.freeHeap) * 100) / espRes.totalHeap;
  esp["cpu_temp"] = round(espRes.cpuTemp * 10) / 10.0;
  esp["cpu_freq_mhz"] = espRes.cpuFreq;
  esp["free_flash"] = espRes.freeFlash;
  
  // Sensor Status
  JsonObject sensors = doc.createNestedObject("sensor_status");
  sensors["voltage_ok"] = sensorStat.voltageOK;
  sensors["current_ok"] = sensorStat.currentOK;
  sensors["oled_ok"] = sensorStat.oledOK;
  sensors["voltage_noise"] = round(sensorStat.voltageNoise * 1000) / 1000.0;
  sensors["current_noise"] = round(sensorStat.currentNoise * 10000) / 10000.0;
  sensors["last_good_ms"] = sensorStat.lastGoodReading;
  
  // Status
  doc["system_status"] = systemStatus;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send HTTP POST
  httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    totalSamplesSent++;
    systemStatus = "API OK";
    
    Serial.printf("✓ API: V=%.1f I=%.3f [%d samples sent]\\n", 
                  avgVoltageRaw, avgCurrentRaw, 
                  bufferFull ? BUFFER_SIZE : samplesCollected);
    
    // Show server response only in verbose mode or if error
    if (verboseMode || (response.indexOf("error") > 0 || response.indexOf("false") > 0)) {
      Serial.printf("Server: %s\\n", response.c_str());
    }
    
  } else {
    systemStatus = "API Failed";
    Serial.printf("✗ API Error: %d\\n", httpResponseCode);
  }
  
  http.end();
  
  // Reset buffer for next cycle  
  samplesCollected = 0;
  bufferFull = false;
  bufferIndex = 0;
}

// --- Update Display ---
void updateDisplay() {
  disp.clearDisplay();
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);
  
  // Header with border
  disp.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  disp.setCursor(28, 2);
  disp.print("API PUBLISHER");
  
  // Connection status
  disp.setCursor(2, 2);
  if (WiFi.status() == WL_CONNECTED) {
    disp.print("W");
    if (httpResponseCode == 200) {
      disp.print("A");
    } else if (httpResponseCode > 0) {
      disp.print("E");
    } else {
      disp.print("X");
    }
  } else {
    disp.print("XX");
  }
  
  // Signal & HTTP code
  disp.setCursor(92, 2);
  disp.printf("%ddBm|%d", wifiRssi, httpResponseCode);
  
  disp.drawLine(2, 12, SCREEN_WIDTH-2, 12, SSD1306_WHITE);
  
  // Current averages
  disp.setCursor(4, 16);
  disp.print("Avg Voltage:");
  disp.setCursor(4, 26);
  disp.printf("%.3f V", avgVoltageRaw);
  
  disp.setCursor(4, 36);
  disp.print("Avg Current:");
  disp.setCursor(4, 46);
  disp.printf("%.4f A", avgCurrentRaw);
  
  // Buffer status
  disp.setCursor(80, 16);
  disp.printf("Buf:%d/%d", samplesCollected, BUFFER_SIZE);
  
  // Calibration (small)
  disp.setCursor(80, 26);
  disp.printf("VC:%.2f", calVolt);
  disp.setCursor(80, 36);
  disp.printf("IC:%.0f", calCurr);
  
  // ESP Resources (compact)
  disp.setCursor(80, 46);
  disp.printf("H:%dK T:%.0fC", espRes.freeHeap/1024, espRes.cpuTemp);
  
  // Sensor status indicators
  disp.setCursor(4, 56);
  disp.printf("%s V:%s I:%s O:%s", 
              systemStatus.substring(0, 3).c_str(),
              sensorStat.voltageOK ? "+" : "-",
              sensorStat.currentOK ? "+" : "-", 
              sensorStat.oledOK ? "+" : "-");
  
  disp.setCursor(80, 56);
  disp.printf("%.1fh", (millis() - bootTime) / 3600000.0);
  
  // Heartbeat indicator
  static bool heartbeat = false;
  heartbeat = !heartbeat;
  if (heartbeat && WiFi.status() == WL_CONNECTED) {
    disp.fillCircle(SCREEN_WIDTH-6, 6, 2, SSD1306_WHITE);
  }
  
  disp.display();
}

// --- Handle Serial Commands ---
void handleSerial() {
  if (!Serial.available()) return;
  
  String input = Serial.readStringUntil('\\n');
  input.trim();
  input.toUpperCase();
  
  if (input.startsWith("V")) {
    // Voltage calibration
    float realV = input.substring(1).toFloat();
    if (realV > 100 && realV < 300) {
      if (avgVoltageRaw > 50) {
        calVolt = realV / avgVoltageRaw;
        prefs.putFloat("calVolt", calVolt);
        Serial.printf("Voltage calibration updated: %.6f\\n", calVolt);
      } else {
        Serial.println("Need valid voltage reading first");
      }
    } else {
      Serial.println("Voltage must be 100-300V. Example: V220");
    }
  }
  else if (input.startsWith("I")) {
    // Current calibration
    float realI = input.substring(1).toFloat();
    if (realI > 0 && realI < 30) {
      if (avgCurrentRaw > 0.001) {
        calCurr *= (realI / avgCurrentRaw);
        emon.current(PIN_CURR, calCurr);
        prefs.putFloat("calCurr", calCurr);
        Serial.printf("Current calibration updated: %.6f\\n", calCurr);
      } else {
        Serial.println("Need valid current reading first");
      }
    } else {
      Serial.println("Current must be 0-30A. Example: I2.5");
    }
  }
  else if (input == "S") {
    // Status
    Serial.println("=== ENERGY METER STATUS ===");
    Serial.printf("Device ID: %s\\n", device_id);
    Serial.printf("API Endpoint: %s\\n", api_endpoint);
    Serial.printf("Average Voltage Raw: %.3f V (cal: %.6f)\\n", avgVoltageRaw, calVolt);
    Serial.printf("Average Current Raw: %.4f A (cal: %.6f)\\n", avgCurrentRaw, calCurr);
    Serial.printf("Buffer: %d/%d samples\\n", samplesCollected, BUFFER_SIZE);
    Serial.printf("WiFi: %s (RSSI: %d dBm)\\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected", wifiRssi);
    Serial.printf("Last HTTP Code: %d\\n", httpResponseCode);
    Serial.printf("Total API Calls: %lu\\n", totalSamplesSent);
    Serial.printf("System Status: %s\\n", systemStatus.c_str());
    Serial.printf("Uptime: %.2f hours\\n", (millis() - bootTime) / 3600000.0);
    Serial.printf("Free Heap: %d bytes\\n", ESP.getFreeHeap());
    Serial.println("===========================");
  }
  else if (input == "T") {
    // Test API
    Serial.println("Testing API connection...");
    sendToAPI();
  }
  else if (input == "V+") {
    // Enable verbose mode
    verboseMode = true;
    Serial.println("✓ Verbose mode ON");
  }
  else if (input == "V-") {
    // Disable verbose mode
    verboseMode = false;
    Serial.println("✓ Verbose mode OFF");
  }
  else if (input == "R") {
    // Restart
    Serial.println("Restarting ESP32...");
    ESP.restart();
  }
  else {
    Serial.println("Commands:");
    Serial.println("  Vxxx - Voltage calibration (e.g. V220)");
    Serial.println("  Ixxx - Current calibration (e.g. I2.5)");
    Serial.println("  S    - System status");
    Serial.println("  T    - Test API");
    Serial.println("  V+   - Verbose mode ON");
    Serial.println("  V-   - Verbose mode OFF");
    Serial.println("  R    - Restart ESP32");
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  bootTime = millis();
  
  Serial.println("=====================================");
  Serial.println("  Energy Meter HTTP API Publisher   ");
  Serial.println("=====================================");
  
  Wire.begin();
  
  // Initialize OLED
  if (!disp.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while(1) delay(1000);
  }
  
  disp.clearDisplay();
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);
  disp.setCursor(30, 30);
  disp.print("Initializing...");
  disp.display();
  delay(1000);
  
  // Load calibration from preferences
  prefs.begin("meter", false);
  calVolt = prefs.getFloat("calVolt", DEFAULT_VOLT_CAL);
  calCurr = prefs.getFloat("calCurr", DEFAULT_CURR_CAL);
  Serial.printf("Loaded calibration: V=%.6f I=%.6f\\n", calVolt, calCurr);
  
  // Setup ADC
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_VOLT, ADC_11db);
  analogSetPinAttenuation(PIN_CURR, ADC_6db);
  
  // Initialize sensors
  voltSensor.setSensitivity(SENSITIVITY);
  emon.current(PIN_CURR, calCurr);
  
  // Connect WiFi
  connectWiFi();
  
  // Sensor warm-up
  disp.clearDisplay();
  disp.setCursor(30, 30);
  disp.print("Warming up...");
  disp.display();
  
  Serial.print("Warming up sensors");
  for (int i = 0; i < 10; i++) {
    voltSensor.getRmsVoltage();
    emon.calcIrms(100);
    Serial.print(".");
    delay(200);
  }
  Serial.println(" Done!");
  
  Serial.printf("API endpoint: %s\\n", api_endpoint);
  Serial.printf("Sample: %lums | Send: %lums | Buffer: %d\\n", SAMPLE_INTERVAL, SEND_INTERVAL, BUFFER_SIZE);
  Serial.println("Ready! Type 'S' for status, 'V+' for verbose mode");
  
  // Initialize timers
  lastSampleTime = millis();
  lastSendTime = millis();
  lastWifiCheck = millis();
}

// --- Main Loop ---
void loop() {
  unsigned long now = millis();
  
  // Check WiFi periodically
  if (now - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
    lastWifiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠ WiFi lost, reconnecting...");
      connectWiFi();
    } else {
      wifiRssi = WiFi.RSSI();
    }
  }
  
  // Sample sensors every second
  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;
    sampleSensors();
  }
  
  // Send averaged data every 5 seconds
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;
    sendToAPI();
  }
  
  // Update display every second
  static unsigned long lastDisplayUpdate = 0;
  if (now - lastDisplayUpdate >= 1000) {
    lastDisplayUpdate = now;
    updateDisplay();
  }
  
  // Handle serial commands
  handleSerial();
  
  // Small delay to prevent watchdog issues
  delay(10);
}