//=============================================================================
// ESP32 Energy Monitor - Data Handler
//=============================================================================

#ifndef DATA_H
#define DATA_H

#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"

// Add DHT library
#include <DHT.h>
#ifndef DHT22_PIN
#define DHT22_PIN 4
#endif
#define DHT_TYPE DHT22
static DHT dht(DHT22_PIN, DHT_TYPE);

//=============================================================================
// DATA STRUCTURES - STEP 3: ADD NEW SENSOR DATA FIELDS HERE
//=============================================================================

// Raw sensor data struct
struct SensorData {
  // ZMPT101B Voltage Sensor
  int zmptRaw;
  float voltage;              // Calculated voltage
  bool zmptActive;
  
  // SCT013 Current Sensor  
  int sctRaw;
  float current;              // Calculated current
  bool sctActive;
  
  // ADD NEW SENSOR FIELDS BELOW:
  // Example: float temperature;
  // Example: float humidity;
  // Example: bool relayStatus;
  // Example: int lightLevel;
  bool pirMotion;             // HC-SR501 PIR motion detected
  
  // DHT22
  float dhtTemperature;       // Celsius
  float dhtHumidity;          // Percent

  // Threshold flags
  bool voltageOutOfRange;     // voltage < VOLT_MIN || voltage > VOLT_MAX
  bool currentOverlimit;      // current > CURRENT_MAX
  bool tempOutOfRange;        // temperature outside TEMP_LOW..TEMP_HIGH
  bool humOutOfRange;         // humidity outside HUM_LOW..HUM_HIGH
};

// System data struct  
struct SystemData {
  unsigned long uptime;
  uint32_t freeHeap;
  uint32_t totalHeap;
  int cpuFreq;
  
  // ADD NEW SYSTEM FIELDS BELOW:
  // Example: float cpuTemp;
  // Example: int wifiReconnects;
};

// WiFi data struct
struct WiFiData {
  String ip;
  String mac;
  int rssi;
  String status;
  
  // ADD NEW WIFI FIELDS BELOW:
  // Example: String gateway;
  // Example: int channel;
};

//=============================================================================
// JSON PAYLOAD HANDLER - STEP 4: ADD NEW SENSORS TO JSON HERE
//=============================================================================

class DataHandler {
private:
  JsonDocument doc;

public:
  DataHandler() {
    doc.to<JsonObject>();
  }

  String createPayload(SensorData sensor, SystemData system, WiFiData wifi) {
    doc.clear();

    // Version and timestamp
    doc["version"] = "1.2";
    doc["ts"] = "2025-09-22T14:20:15Z"; // TODO: Use actual timestamp
    doc["seq"] = 141463; // TODO: Increment sequence number
    doc["tenant"] = "hospital-abc";

    // Device information
    JsonObject device = doc["device"].to<JsonObject>();
    device["id"] = DEVICE_ID;
    device["type"] = "esp32";
    device["fw"] = "2.1.0";
    device["name"] = "IoT Multi-Board A";

    JsonObject location = device["location"].to<JsonObject>();
    location["room"] = "ICU-01";
    location["lat"] = -6.2;
    location["lng"] = 106.8;
    location["alt_m"] = 45;

    JsonArray tags = device["tags"].to<JsonArray>();
    tags.add("demo");
    tags.add("multisensor");
    tags.add("realistic-sim");

    // Network information
    JsonObject network = doc["network"].to<JsonObject>();
    network["conn"] = "wifi";
    network["ip"] = wifi.ip;
    network["rssi_dbm"] = wifi.rssi;
    network["snr_db"] = nullptr; // null
    network["mac"] = wifi.mac;

    // Power information
    JsonObject power = doc["power"].to<JsonObject>();
    power["battery_pct"] = nullptr; // null for wired ESP32
    power["voltage_v"] = 5.0;
    power["charging"] = true;

    // Resources
    JsonObject resources = doc["resources"].to<JsonObject>();
    resources["uptime_s"] = system.uptime;
    resources["cpu_pct"] = 14.2; // TODO: Calculate actual CPU usage
    resources["mem_pct"] = (float)(system.totalHeap - system.freeHeap) / system.totalHeap * 100.0;
    resources["fs_used_pct"] = 68.5; // TODO: Calculate actual flash usage
    resources["heap_free_kb"] = system.freeHeap / 1024;
    resources["flash_free_kb"] = 980; // TODO: Calculate actual flash free
    resources["temp_c"] = 41.8; // TODO: Add CPU temperature sensor

    // Aggregation
    JsonObject agg = doc["agg"].to<JsonObject>();
    agg["window_s"] = 5;
    agg["method"] = "raw";

    // Data array with sensors
    JsonArray dataArray = doc["data"].to<JsonArray>();

    // ZMPT101B Voltage Sensor
    JsonObject zmpt = dataArray.add<JsonObject>();
    zmpt["sensor"] = "zmpt101b";
    zmpt["category"] = "power";
    zmpt["iface"] = "analog";
    zmpt["unit_system"] = "SI";

    JsonObject zmptObs = zmpt["observations"].to<JsonObject>();
    zmptObs["voltage_v"] = sensor.voltage;

    JsonObject zmptQuality = zmpt["quality"].to<JsonObject>();
    zmptQuality["status"] = sensor.zmptActive ? "ok" : "inactive";
    zmptQuality["calibrated"] = true;
    JsonArray zmptErrors = zmptQuality["errors"].to<JsonArray>();
    zmptQuality["notes"] = "AC voltage sensor ZMPT101B for electrical monitoring.";

    // SCT013 Current Sensor
    JsonObject sct = dataArray.add<JsonObject>();
    sct["sensor"] = "sct013";
    sct["category"] = "power";
    sct["iface"] = "analog";
    sct["unit_system"] = "SI";

    JsonObject sctObs = sct["observations"].to<JsonObject>();
    sctObs["current_a"] = sensor.current;

    JsonObject sctQuality = sct["quality"].to<JsonObject>();
    sctQuality["status"] = sensor.sctActive ? "ok" : "inactive";
    sctQuality["calibrated"] = true;
    JsonArray sctErrors = sctQuality["errors"].to<JsonArray>();
    sctQuality["notes"] = "AC current sensor SCT013 for electrical load monitoring.";

    // PIR Motion Sensor
    JsonObject pir = dataArray.add<JsonObject>();
    pir["sensor"] = "hc-sr501";
    pir["category"] = "motion";
    pir["iface"] = "digital";
    pir["unit_system"] = "SI";

    JsonObject pirObs = pir["observations"].to<JsonObject>();
    pirObs["motion_detected"] = sensor.pirMotion;

    JsonObject pirQuality = pir["quality"].to<JsonObject>();
    pirQuality["status"] = "ok";
    pirQuality["calibrated"] = true;
    JsonArray pirErrors = pirQuality["errors"].to<JsonArray>();
    pirQuality["notes"] = "PIR motion sensor HC-SR501 for presence detection.";

    // DHT22 Temperature & Humidity Sensor
    JsonObject dht = dataArray.add<JsonObject>();
    dht["sensor"] = "dht22";
    dht["category"] = "env";
    dht["iface"] = "digital";
    dht["unit_system"] = "SI";

    JsonObject dhtObs = dht["observations"].to<JsonObject>();
    dhtObs["temperature_c"] = sensor.dhtTemperature;
    dhtObs["humidity_pct"] = sensor.dhtHumidity;

    JsonObject dhtQuality = dht["quality"].to<JsonObject>();
    bool dhtValid = !isnan(sensor.dhtTemperature) && !isnan(sensor.dhtHumidity);
    dhtQuality["status"] = dhtValid ? "ok" : "error";
    dhtQuality["calibrated"] = true;
    JsonArray dhtErrors = dhtQuality["errors"].to<JsonArray>();
    if (!dhtValid) {
      dhtErrors.add("sensor_read_failed");
    }
    dhtQuality["notes"] = "DHT22 sensor for room temperature and humidity monitoring.";

    String output;
    serializeJson(doc, output);
    return output;
  }
};

//=============================================================================
// SENSOR READING FUNCTIONS - STEP 5: ADD NEW SENSOR READING CODE HERE
//=============================================================================

// Helper functions for data collection
SensorData readSensors() {
  SensorData data;
  
  //-------------------------------------------------------------------------
  // ZMPT101B (Voltage Sensor) Reading
  //-------------------------------------------------------------------------
  long zmptSum = 0;
  int zmptMax = 0, zmptMin = 4095;
  
  for(int i = 0; i < SAMPLES; i++) {
    int reading = analogRead(ZMPT101B_PIN);
    zmptSum += reading;
    
    // Track min/max for AC signal detection
    if(reading > zmptMax) zmptMax = reading;
    if(reading < zmptMin) zmptMin = reading;
    
    delay(1);
  }
  
  data.zmptRaw = zmptSum / SAMPLES;
  
  // Calculate RMS voltage (simplified)
  int zmptPeakToPeak = zmptMax - zmptMin;
  float zmptVoltage = (zmptPeakToPeak / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
  data.voltage = zmptVoltage * VOLTAGE_CALIBRATION / 2.0; // Convert to RMS
  
  // Check if sensor is active (has AC signal variation)
  data.zmptActive = (zmptPeakToPeak > ZMPT_THRESHOLD);
  
  // Threshold check voltage
  data.voltageOutOfRange = (data.voltage < VOLT_MIN || data.voltage > VOLT_MAX);
  
  //-------------------------------------------------------------------------
  // SCT013 (Current Sensor) Reading
  //-------------------------------------------------------------------------
  long sctSum = 0;
  int sctMax = 0, sctMin = 4095;
  
  for(int i = 0; i < SAMPLES; i++) {
    int reading = analogRead(SCT013_PIN);
    sctSum += reading;
    
    // Track min/max for AC signal detection
    if(reading > sctMax) sctMax = reading;
    if(reading < sctMin) sctMin = reading;
    
    delay(1);
  }
  
  data.sctRaw = sctSum / SAMPLES;
  
  // Calculate RMS current (simplified)
  int sctPeakToPeak = sctMax - sctMin;
  float sctVoltage = (sctPeakToPeak / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
  data.current = sctVoltage * CURRENT_CALIBRATION / 2.0; // Convert to RMS
  
  // Check if sensor is active (has AC signal variation)
  data.sctActive = (sctPeakToPeak > SCT_THRESHOLD);

  // Threshold check current
  data.currentOverlimit = (data.current > CURRENT_MAX);
  
  //-------------------------------------------------------------------------
  // ADD NEW SENSOR READINGS BELOW:
  //-------------------------------------------------------------------------
  
  // PIR HC-SR501 motion sensor (digital input)
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT); // LED indicator
  data.pirMotion = (digitalRead(PIR_PIN) == PIR_ACTIVE_STATE);
  digitalWrite(LED_PIN, data.pirMotion ? LED_ACTIVE_STATE : !LED_ACTIVE_STATE);

  // DHT22 (temperature & humidity)
  // Ensure dht.begin() is called once (init is safe idempotent here)
  static bool dhtInitialized = false;
  if (!dhtInitialized) {
    dht.begin();
    dhtInitialized = true;
  }
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t)) t = NAN; // leave NAN if failed
  if (isnan(h)) h = NAN;
  data.dhtTemperature = t;
  data.dhtHumidity = h;

  // Threshold checks for DHT only when valid
  if (!isnan(t)) {
    data.tempOutOfRange = (t < TEMP_LOW || t > TEMP_HIGH);
  } else {
    data.tempOutOfRange = false;
  }
  if (!isnan(h)) {
    data.humOutOfRange = (h < HUM_LOW || h > HUM_HIGH);
  } else {
    data.humOutOfRange = false;
  }
  
  return data;
}

SystemData getSystemData() {
  SystemData data;
  data.uptime = millis() / 1000;
  data.freeHeap = ESP.getFreeHeap();
  data.totalHeap = ESP.getHeapSize();
  data.cpuFreq = ESP.getCpuFreqMHz();
  return data;
}

WiFiData getWiFiData() {
  WiFiData data;
  data.ip = WiFi.localIP().toString();
  data.mac = WiFi.macAddress();
  data.rssi = WiFi.RSSI();
  data.status = WiFi.status() == WL_CONNECTED ? "connected" : "disconnected";
  return data;
}

// Function for real-time PIR reading and LED control
void readPirRealtime() {
  bool motion = (digitalRead(PIR_PIN) == PIR_ACTIVE_STATE);
  digitalWrite(LED_PIN, motion ? LED_ACTIVE_STATE : !LED_ACTIVE_STATE);
}

#endif
