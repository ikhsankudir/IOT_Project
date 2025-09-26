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
    // Tidak perlu tampilkan JSON di serial
  }
  
  static void printHTTP(int code) {
    if (DEBUG_ENABLED && Serial) {
      Serial.println(code == 200 ? "HTTP: OK" : "HTTP: ERR");
    }
  }

  static void printSummary(const SensorData& sensor, const SystemData& system, const WiFiData& wifi) {
    if (!DEBUG_ENABLED || !Serial) return;
    Serial.println(F("==== STATUS ENERGI ===="));
    Serial.print(F("V: ")); Serial.print(sensor.pzemVoltage, 1); Serial.print(F("V  "));
    Serial.print(F("I: ")); Serial.print(sensor.pzemCurrent, 2); Serial.print(F("A  "));
    Serial.print(F("P: ")); Serial.print(sensor.pzemPower, 1); Serial.print(F("W  "));
    Serial.print(F("E: ")); Serial.print(sensor.pzemEnergy, 3); Serial.println(F("kWh"));
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
    // Header with status
    //-------------------------------------------------------------------------
    display.print("ESP32 Monitor");
    display.setCursor(100, 0);
    display.print(wifi.status == "connected" ? "W" : "X");
    display.print(httpOK ? "H" : "X");

    //-------------------------------------------------------------------------
    // Power readings
    //-------------------------------------------------------------------------
    display.setCursor(0, 9);
    display.printf("V:%.1fV I:%.2fA", sensor.pzemVoltage, sensor.pzemCurrent);

    //-------------------------------------------------------------------------
    // Environment readings
    //-------------------------------------------------------------------------
    display.setCursor(0, 18);
    if (!isnan(sensor.dhtTemperature) && !isnan(sensor.dhtHumidity)) {
      display.printf("T:%.1fC H:%d%%", sensor.dhtTemperature, (int)sensor.dhtHumidity);
    } else {
      display.print("T:-- H:--");
    }

    //-------------------------------------------------------------------------
    // Motion sensors
    //-------------------------------------------------------------------------
    display.setCursor(0, 27);
    display.print("PIR:");
    display.print(sensor.pirMotion ? "YES" : "NO");
    display.setCursor(50, 27);
    display.print("IR:");
    display.print(sensor.irDetected ? "YES" : "NO");

    //-------------------------------------------------------------------------
    // System info
    //-------------------------------------------------------------------------
    display.setCursor(0, 36);
    display.printf("RAM:%dKB Up:%ds", system.freeHeap / 1024, system.uptime);

    //-------------------------------------------------------------------------
    // Status indicators
    //-------------------------------------------------------------------------
    display.setCursor(0, 45);
    display.print("PWR:");
    display.print(sensor.pzemActive ? "OK" : "ERR");

    display.setCursor(0, 54);
    if (sensor.voltageOutOfRange || sensor.currentOverlimit) {
      display.print("ALERT: ");
      if (sensor.voltageOutOfRange) display.print("V ");
      if (sensor.currentOverlimit) display.print("I ");
    } else {
      display.print("STATUS: NORMAL");
    }

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
