//=============================================================================
// ESP32 Energy Monitor - Display & Debug Handler  
//=============================================================================

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "config.h"
#include "data.h"

// Forward declarations
void debugPrintln(const char* message);

class DebugHandler {
public:
  static void init() {
    if (DEBUG_ENABLED) {
      Serial.begin(SERIAL_BAUD);
      while (!Serial && millis() < 2000);
    }
  }
  
  static void println(const char* message) {
    if (DEBUG_ENABLED && Serial) {
      Serial.println(message);
    }
  }
  
  static void printJson(const String& json) {
    // No need to display JSON in serial
  }
  
  static void printHTTP(int code) {
    if (DEBUG_ENABLED && Serial) {
      Serial.println(code == 200 ? "HTTP: OK" : "HTTP: ERR");
    }
  }

  static void printSummary(const SensorData& sensor, const SystemData& system, const WiFiData& wifi) {
    if (!DEBUG_ENABLED || !Serial) return;
    Serial.println(F("==== ENERGY STATUS ===="));
    Serial.print(F("V: ")); Serial.print(sensor.voltage, 1); Serial.print(F("V  "));
    Serial.print(F("I: ")); Serial.print(sensor.current, 2); Serial.print(F("A  "));
    Serial.print(F("PIR: ")); Serial.print(sensor.pirMotion ? "MOTION" : "IDLE");
    Serial.println();
    Serial.print(F("T: "));
    if (!isnan(sensor.dhtTemperature)) Serial.print(sensor.dhtTemperature, 1); else Serial.print("-");
    Serial.print(F("C  H: "));
    if (!isnan(sensor.dhtHumidity)) Serial.print(sensor.dhtHumidity, 0); else Serial.print("-");
    Serial.println(F("%"));
    Serial.print(F("WiFi: ")); Serial.print(wifi.status); Serial.print(F("  RSSI: ")); Serial.println(wifi.rssi);
    Serial.print(F("Uptime: ")); Serial.print(system.uptime); Serial.print(F("s  RAM: ")); Serial.print(system.freeHeap / 1024); Serial.println(F("KB"));
    Serial.println(F("======================"));
  }
};

//=============================================================================
// DISPLAY HANDLER - STEP 6: MODIFY DISPLAY LAYOUT FOR NEW SENSORS HERE
//=============================================================================

class DisplayHandler {
private:
  Adafruit_SSD1306 display;

public:
  DisplayHandler() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}
  
  bool init() {
    Wire.begin(SDA_PIN, SCL_PIN);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      debugPrintln("OLED FAIL");
      return false;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.display();
    debugPrintln("OLED OK");
    return true;
  }
  
  void update(SensorData sensor, SystemData system, WiFiData wifi, bool httpOK) {
    display.clearDisplay();
    display.setCursor(0, 0);
    
    //-------------------------------------------------------------------------
    // Header with status indicators
    //-------------------------------------------------------------------------
    display.print("ESP32 Monitor");
    display.setCursor(100, 0);
    display.print(wifi.status == "connected" ? "W" : "X");  // WiFi status
    display.print(httpOK ? "H" : "X");                      // HTTP status
    
    // Line separator
    display.setCursor(0, 10);
    display.println("----------------");
    
    //-------------------------------------------------------------------------
    // Current Sensors Display
    //-------------------------------------------------------------------------
    // ZMPT Voltage Sensor
    display.print("V: ");
    display.print(sensor.voltage, 0);
    display.print("V ");
    display.println(sensor.zmptActive ? "ON" : "OFF");
    
    // SCT Current Sensor
    display.print("I: ");
    display.print(sensor.current, 1);
    display.print("A ");
    display.println(sensor.sctActive ? "ON" : "OFF");
    
    //-------------------------------------------------------------------------
    // ADD NEW SENSOR DISPLAYS BELOW:
    //-------------------------------------------------------------------------
    
    // PIR Motion
    display.print("PIR: ");
    display.println(sensor.pirMotion ? "MOTION" : "IDLE");
    
    // Example: Temperature & Humidity
    // display.print("T: ");
    // display.print(sensor.temperature, 1);
    // display.println("C");
    //
    // display.print("H: ");
    // display.print(sensor.humidity, 0);
    // display.println("%");
    
    // Example: Digital status
    // display.print("Relay: ");
    // display.println(sensor.relayStatus ? "ON" : "OFF");
    
    //-------------------------------------------------------------------------
    // System Information (Keep at bottom)
    //-------------------------------------------------------------------------
    display.print("RAM: ");
    display.print(system.freeHeap / 1024);
    display.println("KB");
    
    display.print("Up: ");
    display.print(system.uptime);
    display.println("s");
    
    display.display();
  }
  
  void showStartup() {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.println("ESP32");
    display.setTextSize(1);
    display.println("Starting...");
    display.display();
  }
};

// Global debug function implementations
void debugPrintln(const char* message) {
  DebugHandler::println(message);
}

#endif
