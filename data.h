//=============================================================================
// ESP32 Energy Monitor - Data Handler
//=============================================================================

#ifndef DATA_H
#define DATA_H

#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"

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
    
    //-------------------------------------------------------------------------
    // Source ESP - System information
    //-------------------------------------------------------------------------
    JsonObject esp = doc["source_esp"].to<JsonObject>();
    esp["device_id"] = DEVICE_ID;
    esp["uptime"] = system.uptime;
    esp["free_heap"] = system.freeHeap;
    esp["total_heap"] = system.totalHeap;
    esp["cpu_freq"] = system.cpuFreq;
    
    // ADD NEW SYSTEM FIELDS TO JSON BELOW:
    // Example: esp["cpu_temp"] = system.cpuTemp;
    
    //-------------------------------------------------------------------------
    // Sensors - All sensor readings
    //-------------------------------------------------------------------------
    JsonObject sensors = doc["sensors"].to<JsonObject>();
    
    // ZMPT101B Voltage Sensor
    JsonObject zmpt = sensors["zmpt101b"].to<JsonObject>();
    zmpt["raw"] = sensor.zmptRaw;
    zmpt["voltage"] = sensor.voltage;
    zmpt["active"] = sensor.zmptActive;
    
    // SCT013 Current Sensor
    JsonObject sct = sensors["sct013"].to<JsonObject>();
    sct["raw"] = sensor.sctRaw;
    sct["current"] = sensor.current;
    sct["active"] = sensor.sctActive;
    
    // ADD NEW SENSORS TO JSON BELOW:
    // Example:
    // JsonObject dht = sensors["dht22"].to<JsonObject>();
    // dht["temperature"] = sensor.temperature;
    // dht["humidity"] = sensor.humidity;
    //
    // JsonObject relay = sensors["relay"].to<JsonObject>();
    // relay["status"] = sensor.relayStatus;
    
    //-------------------------------------------------------------------------
    // WiFi information
    //-------------------------------------------------------------------------
    JsonObject wifiObj = doc["wifi"].to<JsonObject>();
    wifiObj["ip"] = wifi.ip;
    wifiObj["mac"] = wifi.mac;
    wifiObj["rssi"] = wifi.rssi;
    wifiObj["status"] = wifi.status;
    
    // ADD NEW WIFI FIELDS TO JSON BELOW:
    // Example: wifiObj["gateway"] = wifi.gateway;
    
    //-------------------------------------------------------------------------
    // Timestamp
    //-------------------------------------------------------------------------
    doc["timestamp"] = millis();
    
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
  
  //-------------------------------------------------------------------------
  // ADD NEW SENSOR READINGS BELOW:
  //-------------------------------------------------------------------------
  
  // Example: DHT22 Temperature & Humidity Sensor
  // data.temperature = dht.readTemperature();
  // data.humidity = dht.readHumidity();
  
  // Example: Digital sensors
  // data.relayStatus = digitalRead(RELAY_PIN);
  
  // Example: Analog sensors  
  // data.lightLevel = analogRead(LIGHT_SENSOR_PIN);
  
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

#endif
