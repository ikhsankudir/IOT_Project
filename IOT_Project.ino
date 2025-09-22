//=============================================================================
// ESP32 Energy Monitor - Main Program
//=============================================================================
// 
// STEP 7: ADD LIBRARY INCLUDES FOR NEW SENSORS HERE
// Example: #include <DHT.h>
// Example: #include <Servo.h>
//
//=============================================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "data.h"
#include "display.h"

//=============================================================================
// GLOBAL OBJECTS - STEP 8: ADD NEW SENSOR OBJECTS HERE
//=============================================================================

// System objects
DisplayHandler displayHandler;
DataHandler dataHandler;
HTTPClient http;

// ADD NEW SENSOR OBJECTS BELOW:
// Example: DHT dht(DHT22_PIN, DHT22);
// Example: Servo myServo;

// Timing & status variables
unsigned long lastSend = 0, lastDisplay = 0;
bool httpStatus = false;

// Current readings
SensorData currentSensor;
SystemData currentSystem;
WiFiData currentWiFi;

//=============================================================================
// SETUP FUNCTION - STEP 9: ADD NEW SENSOR INITIALIZATION HERE  
//=============================================================================

void setup() {
  DebugHandler::init();
  debugPrintln("Starting ESP32...");
  
  // Initialize display
  if (!displayHandler.init()) {
    debugPrintln("Display failed");
  }
  displayHandler.showStartup();
  
  //-------------------------------------------------------------------------
  // ADD NEW SENSOR INITIALIZATION BELOW:
  //-------------------------------------------------------------------------
  
  // Example: DHT sensor
  // dht.begin();
  
  // Example: Pin modes for digital sensors
  // pinMode(RELAY_PIN, OUTPUT);
  // pinMode(LED_PIN, OUTPUT);
  
  // Example: Servo initialization
  // myServo.attach(SERVO_PIN);
  
  //-------------------------------------------------------------------------
  // WiFi connection (keep at end)
  //-------------------------------------------------------------------------
  connectWiFi();
  debugPrintln("Ready");
}

void loop() {
  // Read sensors
  currentSensor = readSensors();
  currentSystem = getSystemData();
  currentWiFi = getWiFiData();
  
  // Update display
  if (millis() - lastDisplay >= DISPLAY_UPDATE) {
    displayHandler.update(currentSensor, currentSystem, currentWiFi, httpStatus);
    lastDisplay = millis();
  }
  
  // Send data
  if (millis() - lastSend >= SEND_INTERVAL && WiFi.status() == WL_CONNECTED) {
    sendData();
    lastSend = millis();
  }
  
  // Auto-reconnect WiFi
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  
  delay(50);
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    timeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    debugPrintln("WiFi OK");
  } else {
    debugPrintln("WiFi FAIL");
  }
}

void sendData() {
  String payload = dataHandler.createPayload(currentSensor, currentSystem, currentWiFi);
  
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-ID", DEVICE_ID);
  
  DebugHandler::printJson(payload);
  
  int code = http.POST(payload);
  httpStatus = (code == 200);
  
  DebugHandler::printHTTP(code);
  http.end();
}
