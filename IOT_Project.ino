//=============================================================================
// ESP32 Energy Monitor - Main Program (Dual Core with FreeRTOS Tasks)
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

// Timing & status variables (shared state)
unsigned long lastSend = 0, lastDisplay = 0;
bool httpStatus = false;

// Current readings (shared state)
SensorData currentSensor;
SystemData currentSystem;
WiFiData currentWiFi;

//=============================================================================
// FreeRTOS: Task Handles & Synchronization
//=============================================================================
TaskHandle_t taskSensorHandle = NULL;
TaskHandle_t taskDisplayHandle = NULL;
TaskHandle_t taskNetworkHandle = NULL;

SemaphoreHandle_t dataMutex;  // protect currentSensor/currentSystem/currentWiFi/httpStatus

// Forward declarations
void connectWiFi();
void sendData();

// Task functions
void taskSensor(void* pvParameters) {
  (void) pvParameters;
  for (;;) {
    SensorData sensor = readSensors();
    SystemData system = getSystemData();
    WiFiData wifi = getWiFiData();

    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      currentSensor = sensor;
      currentSystem = system;
      currentWiFi = wifi;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void taskDisplay(void* pvParameters) {
  (void) pvParameters;
  for (;;) {
    bool localHttpOK;
    SensorData sensor;
    SystemData system;
    WiFiData wifi;

    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      sensor = currentSensor;
      system = currentSystem;
      wifi = currentWiFi;
      localHttpOK = httpStatus;
      xSemaphoreGive(dataMutex);
    }

    displayHandler.update(sensor, system, wifi, localHttpOK);
    DebugHandler::printSummary(sensor, system, wifi);
    vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE));
  }
}

void taskNetwork(void* pvParameters) {
  (void) pvParameters;
  unsigned long lastWifiCheck = 0;
  for (;;) {
    // WiFi reconnect check
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    } else {
      // send payload periodically
      static unsigned long lastTx = 0;
      unsigned long now = millis();
      if (now - lastTx >= SEND_INTERVAL) {
        // Prepare safe local copies
        SensorData sensor;
        SystemData system;
        WiFiData wifi;
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
          sensor = currentSensor;
          system = currentSystem;
          wifi = currentWiFi;
          xSemaphoreGive(dataMutex);
        }

        String payload = dataHandler.createPayload(sensor, system, wifi);

        http.begin(API_ENDPOINT);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-Device-Id", DEVICE_ID);
        // http.addHeader("X-Api-Key", API_KEY); // Commented out for dev API (no security)

        DebugHandler::printJson(payload);
        int code = http.POST(payload);
        bool ok = (code == 200);
        DebugHandler::printHTTP(code);
        http.end();

        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
          httpStatus = ok;
          xSemaphoreGive(dataMutex);
        }

        lastTx = now;
      }
    }

    // periodic WiFi RSSI refresh for UI even if not sending
    if (millis() - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
      if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        currentWiFi = getWiFiData();
        xSemaphoreGive(dataMutex);
      }
      lastWifiCheck = millis();
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

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

  // Init shared state
  currentSensor = SensorData{};
  currentSystem = getSystemData();
  currentWiFi = getWiFiData();

  // Create mutex
  dataMutex = xSemaphoreCreateMutex();

  // Create tasks pinned to cores
  // Core assignment suggestion:
  // - Core 1: Sensor + Display (I/O + light work)
  // - Core 0: Network/HTTP (WiFi stack runs on core 0)
  xTaskCreatePinnedToCore(taskSensor,  "TASK_SENSOR",  4096, NULL, 2, &taskSensorHandle, 1);
  xTaskCreatePinnedToCore(taskDisplay, "TASK_DISPLAY", 4096, NULL, 1, &taskDisplayHandle, 1);
  xTaskCreatePinnedToCore(taskNetwork, "TASK_NET",     8192, NULL, 2, &taskNetworkHandle, 0);

  debugPrintln("Ready");
}

void loop() {
  readPirRealtime();
  vTaskDelay(pdMS_TO_TICKS(1)); // loop cepat untuk PIR
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

// Deprecated in tasking mode: kept for reference
void sendData() {
  SensorData sensor;
  SystemData system;
  WiFiData wifi;
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    sensor = currentSensor;
    system = currentSystem;
    wifi = currentWiFi;
    xSemaphoreGive(dataMutex);
  }

  String payload = dataHandler.createPayload(sensor, system, wifi);
  
  http.begin(API_ENDPOINT);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-ID", DEVICE_ID);
  
  DebugHandler::printJson(payload);
  
  int code = http.POST(payload);
  bool ok = (code == 200);
  
  DebugHandler::printHTTP(code);
  http.end();

  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    httpStatus = ok;
    xSemaphoreGive(dataMutex);
  }
}