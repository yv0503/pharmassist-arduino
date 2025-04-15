#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <EEPROM.h>
#include <WiFiS3.h>
#include <local/BLELocalDevice.h>

#include "constants.h"
#include "led_matrix/bluetooth_matrix.h"
#include "led_matrix/wifi_matrix.h"
#include "setup/wifi_credentials.h"
#include "setup/bluetooth.h"
#include "setup/wifi_connection.h"
#include "networking/web_server.h"

// Arduino UNO R4 WiFi

#define resetPin 12

ArduinoLEDMatrix matrix;
byte isInitialized = 0;

String ssid = "";
String password = "";
unsigned long previousMillis = 0;
constexpr long interval = 1000;
byte currentFrame = 0;
bool wasConnected = false;

const String testServer = "www.google.com";
int wifiStatus = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  Serial.println("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
  Serial.println("┃               PharmAssist                 ┃");
  Serial.println("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");
  EEPROM.begin();
  matrix.begin();
  isInitialized = EEPROM.read(isInitializedAddress);

  if (isInitialized != 1) {
    Serial.println("Device not initialized. Starting Bluetooth setup...");
    setupBluetooth(ssid, password);
    matrix.loadFrame(bluetooth_matrix[0]);

    while (isInitialized != 1) {
      bluetoothLoop();

      const bool isConnected = isBleConnected();

      if (const unsigned long currentMillis = millis(); currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        if (!isConnected) {
          currentFrame = 1 - currentFrame;
          matrix.loadFrame(bluetooth_matrix[currentFrame]);
        } else if (!wasConnected) {
          matrix.loadFrame(bluetooth_matrix[1]);
          wasConnected = true;
        }
      }

      if (!isConnected) wasConnected = false;

      isInitialized = EEPROM.read(isInitializedAddress);
    }

    BLE.end();
    Serial.println("Bluetooth setup complete");
    matrix.clear();
  }

  loadWiFiCredentials(ssid, password);
  wifiStatus = connectToWiFi(ssid, password, matrix);

  // Initialize web server after WiFi connection is established
  if (wifiStatus == WL_CONNECTED) {
    setupWebServer();
    Serial.print("Web server available at http://");
    Serial.println(WiFi.localIP());
  }

  delay(3000);
  matrix.clear();
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    wifiStatus = connectToWiFi(ssid, password, matrix);
  }

  if (wifiStatus == WL_CONNECTED) {
    handleWebServerClients();
  }
}

