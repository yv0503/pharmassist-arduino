#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <BLECharacteristic.h>
#include <ArduinoJson.h>
#include <Arduino_LED_Matrix.h>

#include "../constants.h"
#include "../led_matrix/bluetooth_matrix.h"
#include "bluetooth.h"
#include "wifi_credentials.h"

BLEService bluetoothService(WIFI_SERVICE_UUID.c_str());
BLEStringCharacteristic wifiJsonCharacteristic(WIFI_JSON_CHARACTERISTIC_UUID.c_str(), BLEWrite | BLERead, JSON_BUFFER_SIZE);

// References to external variables
String* ssidPtr = nullptr;
String* passwordPtr = nullptr;
bool setupComplete = false;

// ReSharper disable CppParameterNeverUsed
void onJsonReceived(BLEDevice central, BLECharacteristic characteristic) { // NOLINT(*-unnecessary-value-param)
  String jsonStr = wifiJsonCharacteristic.value();
  Serial.print("Received JSON: ");
  Serial.println(jsonStr);
  
  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, jsonStr);
  
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  if (!doc["ssid"].isNull() && !doc["password"].isNull()) {
    *ssidPtr = doc["ssid"].as<String>();
    *passwordPtr = doc["password"].as<String>();

    Serial.print("SSID received: ");
    Serial.println(*ssidPtr);
    Serial.print("Password length: ");
    Serial.println(passwordPtr->length());

    if (ssidPtr->length() > 0 && passwordPtr->length() > 0) {
      saveWiFiCredentials(*ssidPtr, *passwordPtr);
      EEPROM.write(isInitializedAddress, 1);
      setupComplete = true;

      // Send confirmation response back
      JsonDocument response;
      response["status"] = "success";
      String responseStr;
      serializeJson(response, responseStr);
      wifiJsonCharacteristic.writeValue(responseStr);

      Serial.println("Credentials saved, BLE setup complete");
    } else {
      Serial.println("Error: SSID or password is empty");
    }
  } else {
    Serial.println("Error: JSON missing required fields");
  }
}

// ReSharper disable once CppPassValueParameterByConstReference
void onBLEConnected(BLEDevice central) { // NOLINT(*-unnecessary-value-param)
  Serial.print("Connected to central: ");
  Serial.println(central.address());
}

// ReSharper disable once CppPassValueParameterByConstReference
void onBLEDisconnected(BLEDevice central) { // NOLINT(*-unnecessary-value-param)
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());

  if (!setupComplete) {
    Serial.println("Setup was not completed. Restarting advertising...");
    BLE.advertise();
  }
}

void setupBluetooth(String& ssid, String& password) {
  ssidPtr = &ssid;
  passwordPtr = &password;
  setupComplete = false;

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (true);
  }

  // Make device discoverable
  BLE.setDeviceName(DEVICE_NAME.c_str());
  BLE.setLocalName(DEVICE_NAME.c_str());
  BLE.setAdvertisedService(bluetoothService);

  // Add the JSON characteristic
  bluetoothService.addCharacteristic(wifiJsonCharacteristic);

  // Set up the event handlers
  wifiJsonCharacteristic.setEventHandler(BLEWritten, onJsonReceived);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
  BLE.setEventHandler(BLEConnected, onBLEConnected);

  // Add service and start advertising
  BLE.addService(bluetoothService);
  BLE.advertise();

  Serial.println("Bluetooth® device active, waiting for connections...");
}

void bluetoothLoop() {
  BLE.poll();

  if (setupComplete) {
    delay(1000);
    BLE.stopAdvertise();
  }
}

bool isBleConnected() {
  return BLE.connected();
}

void runBluetoothSetup(String& ssid, String& password, ArduinoLEDMatrix& matrix) {
  Serial.println("Starting Bluetooth setup...");
  setupBluetooth(ssid, password);
  matrix.begin();
  matrix.loadFrame(bluetooth_matrix[0]);
  
  bool wasConnected = false;
  
  unsigned long previousMillis = 0;
  byte currentFrame = 0;

  while (EEPROM.read(isInitializedAddress) != 1) {
    constexpr long interval = 1000;
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
  }

  BLE.end();
  Serial.println("Bluetooth setup complete");
  matrix.clear();
}

