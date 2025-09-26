//=============================================================================
// ESP32 Energy Monitor - Data Handler
//=============================================================================

#ifndef DATA_H
#define DATA_H

#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"

// Tambahkan library DHT
#include <DHT.h>
#ifndef DHT22_PIN
#define DHT22_PIN 4
#endif
#define DHT_TYPE DHT22
static DHT dht(DHT22_PIN, DHT_TYPE);

// PZEM-004T extern declaration
extern PZEM004Tv30 pzem;

//=============================================================================
// DATA STRUCTURES - STEP 3: ADD NEW SENSOR DATA FIELDS HERE
//=============================================================================

// Raw sensor data struct
struct SensorData {
  // ZMPT101B Voltage Sensor (commented out - replaced by PZEM)
  // int zmptRaw;
  // float voltage;              // Calculated voltage
  // bool zmptActive;

  // SCT013 Current Sensor (commented out - replaced by PZEM)
  // int sctRaw;
  // float current;              // Calculated current
  // bool sctActive;

  // PZEM-004T Power Meter
  float pzemVoltage;          // Voltage (V)
  float pzemCurrent;          // Current (A)
  float pzemPower;            // Power (W)
  float pzemEnergy;           // Energy (kWh)
  float pzemFrequency;        // Frequency (Hz)
  float pzemPowerFactor;      // Power Factor
  bool pzemActive;            // PZEM communication status

  // ADD NEW SENSOR FIELDS BELOW:
  // Example: float temperature;
  // Example: float humidity;
  // Example: bool relayStatus;
  // Example: int lightLevel;
  bool pirMotion;             // HC-SR501 PIR motion detected
  
  // DHT22
  float dhtTemperature;       // Celsius
  float dhtHumidity;          // Percent

  // IR Proximity Sensor
  bool irDetected;            // IR obstacle detected

  // RCWL-0516 Microwave Radar Sensor
  bool rcwlMotion;            // RCWL-0516 motion detected

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

    // PZEM-004T Power Meter
    JsonObject pzem = dataArray.add<JsonObject>();
    pzem["sensor"] = "pzem-004t";
    pzem["category"] = "power";
    pzem["iface"] = "serial";
    pzem["unit_system"] = "SI";

    JsonObject pzemObs = pzem["observations"].to<JsonObject>();
    pzemObs["voltage_v"] = sensor.pzemVoltage;
    pzemObs["current_a"] = sensor.pzemCurrent;
    pzemObs["power_w"] = sensor.pzemPower;
    pzemObs["energy_kwh"] = sensor.pzemEnergy;
    pzemObs["frequency_hz"] = sensor.pzemFrequency;
    pzemObs["power_factor"] = sensor.pzemPowerFactor;

    JsonObject pzemQuality = pzem["quality"].to<JsonObject>();
    pzemQuality["status"] = sensor.pzemActive ? "ok" : "error";
    pzemQuality["calibrated"] = true;
    JsonArray pzemErrors = pzemQuality["errors"].to<JsonArray>();
    if (!sensor.pzemActive) {
      pzemErrors.add("communication_failed");
    }
    pzemQuality["notes"] = "PZEM-004T power meter dengan split CT untuk monitoring listrik komprehensif.";

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
    pirQuality["notes"] = "Sensor gerak PIR HC-SR501 untuk deteksi kehadiran.";

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
    dhtQuality["notes"] = "Sensor DHT22 untuk monitoring suhu dan kelembapan ruangan.";

    // IR Proximity Sensor
    JsonObject ir = dataArray.add<JsonObject>();
    ir["sensor"] = "ir-proximity";
    ir["category"] = "proximity";
    ir["iface"] = "digital";
    ir["unit_system"] = "SI";

    JsonObject irObs = ir["observations"].to<JsonObject>();
    irObs["obstacle_detected"] = sensor.irDetected;

    JsonObject irQuality = ir["quality"].to<JsonObject>();
    irQuality["status"] = "ok";
    irQuality["calibrated"] = true;
    JsonArray irErrors = irQuality["errors"].to<JsonArray>();
    irQuality["notes"] = "Sensor IR proximity untuk deteksi obstacle/hambatan.";

    // RCWL-0516 Microwave Radar Sensor
    JsonObject rcwl = dataArray.add<JsonObject>();
    rcwl["sensor"] = "rcwl-0516";
    rcwl["category"] = "motion";
    rcwl["iface"] = "digital";
    rcwl["unit_system"] = "SI";

    JsonObject rcwlObs = rcwl["observations"].to<JsonObject>();
    rcwlObs["motion_detected"] = sensor.rcwlMotion;

    JsonObject rcwlQuality = rcwl["quality"].to<JsonObject>();
    rcwlQuality["status"] = "ok";
    rcwlQuality["calibrated"] = true;
    JsonArray rcwlErrors = rcwlQuality["errors"].to<JsonArray>();
    rcwlQuality["notes"] = "Sensor radar gelombang mikro RCWL-0516 untuk deteksi gerakan.";

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
  // PZEM-004T Power Meter Reading
  //-------------------------------------------------------------------------
  static int debugCount = 0;
  debugCount++;
  bool shouldDebug = (DEBUG_ENABLED && (debugCount % 20 == 0)); // Print every 20 readings (~1 second)

  if (shouldDebug) Serial.println("Reading PZEM...");
  // Read all parameters from PZEM-004T
  data.pzemVoltage = pzem.voltage();
  data.pzemCurrent = pzem.current();
  data.pzemPower = pzem.power();
  data.pzemEnergy = pzem.energy();
  data.pzemFrequency = pzem.frequency();
  data.pzemPowerFactor = pzem.pf();

  // Check if PZEM communication is successful (voltage > 0 indicates valid reading)
  data.pzemActive = (data.pzemVoltage > 0 && !isnan(data.pzemVoltage));
  if (!data.pzemActive) {
    if (shouldDebug) Serial.println("PZEM communication failed");
  } else {
    if (shouldDebug) Serial.printf("PZEM OK: V=%.1f I=%.2f P=%.1f E=%.3f\n", data.pzemVoltage, data.pzemCurrent, data.pzemPower, data.pzemEnergy);
  }

  // Threshold checks for PZEM data
  if (data.pzemActive) {
    data.voltageOutOfRange = (data.pzemVoltage < VOLT_MIN || data.pzemVoltage > VOLT_MAX);
    data.currentOverlimit = (data.pzemCurrent > CURRENT_MAX);
  } else {
    data.voltageOutOfRange = false;
    data.currentOverlimit = false;
  }

  //-------------------------------------------------------------------------
  // ZMPT101B (Voltage Sensor) Reading - COMMENTED OUT
  //-------------------------------------------------------------------------
  /*
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
  */

  //-------------------------------------------------------------------------
  // SCT013 (Current Sensor) Reading - COMMENTED OUT
  //-------------------------------------------------------------------------
  /*
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
  */
  
  //-------------------------------------------------------------------------
  // ADD NEW SENSOR READINGS BELOW:
  //-------------------------------------------------------------------------
  
  // PIR HC-SR501 motion sensor (digital input) with false trigger prevention
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT); // LED indikator

  // False trigger prevention - read multiple times and use majority
  int pirReadings = 0;
  for(int i = 0; i < 3; i++) {
    if(digitalRead(PIR_PIN) == PIR_ACTIVE_STATE) pirReadings++;
    delay(5); // Small delay between readings
  }
  data.pirMotion = (pirReadings >= 2); // Majority vote: at least 2 out of 3 readings must be active

  digitalWrite(LED_PIN, data.pirMotion ? LED_ACTIVE_STATE : !LED_ACTIVE_STATE);

  // DHT22 (temperature & humidity)
  // Pastikan dht.begin() sudah dipanggil sekali (init aman idempotent di sini)
  static bool dhtInitialized = false;
  if (!dhtInitialized) {
    dht.begin();
    dhtInitialized = true;
  }
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t)) t = NAN; // biarkan NAN jika gagal
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

  // IR Proximity Sensor (digital input) with debouncing
  pinMode(IR_PIN, INPUT);

  // Simple debouncing - read multiple times and use majority
  int irReadings = 0;
  for(int i = 0; i < 5; i++) {
    if(digitalRead(IR_PIN) == LOW) irReadings++;
    delay(10); // Small delay between readings
  }
  data.irDetected = (irReadings >= 3); // Majority vote: at least 3 out of 5 readings must be LOW

  // RCWL-0516 Microwave Radar Sensor (digital input) with false trigger prevention
  pinMode(RCWL0516_PIN, INPUT);
  pinMode(RCWL_LED_PIN, OUTPUT); // RCWL LED indicator

  // False trigger prevention - read multiple times and use majority
  int rcwlReadings = 0;
  for(int i = 0; i < 3; i++) {
    if(digitalRead(RCWL0516_PIN) == HIGH) rcwlReadings++;
    delay(5); // Small delay between readings
  }
  data.rcwlMotion = (rcwlReadings >= 2); // Majority vote: at least 2 out of 3 readings must be HIGH

  digitalWrite(RCWL_LED_PIN, data.rcwlMotion ? LED_ACTIVE_STATE : !LED_ACTIVE_STATE);

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

// Fungsi pembacaan PIR dan kontrol LED secara realtime dengan false trigger prevention
void readPirRealtime() {
  // False trigger prevention - quick majority vote
  int pirReadings = 0;
  for(int i = 0; i < 3; i++) {
    if(digitalRead(PIR_PIN) == PIR_ACTIVE_STATE) pirReadings++;
    delay(2); // Very short delay for realtime response
  }
  bool motion = (pirReadings >= 2); // At least 2 out of 3 readings
  digitalWrite(LED_PIN, motion ? LED_ACTIVE_STATE : !LED_ACTIVE_STATE);
}

// Fungsi pembacaan RCWL-0516 dan kontrol LED secara realtime dengan false trigger prevention
void readRcwlRealtime() {
  // False trigger prevention - quick majority vote
  int rcwlReadings = 0;
  for(int i = 0; i < 3; i++) {
    if(digitalRead(RCWL0516_PIN) == HIGH) rcwlReadings++;
    delay(2); // Very short delay for realtime response
  }
  bool motion = (rcwlReadings >= 2); // At least 2 out of 3 readings
  digitalWrite(RCWL_LED_PIN, motion ? LED_ACTIVE_STATE : !LED_ACTIVE_STATE);
}

#endif
