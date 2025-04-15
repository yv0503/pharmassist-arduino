#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <BLECharacteristic.h>
#include <ArduinoJson.h>

#include "bluetooth.h"
#include "../constants.h"
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
  // Store references to the external variables
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

  // Check if setup is complete to allow main loop to detect completion
  if (setupComplete) {
    delay(1000); // Give time for confirmation to be sent
    BLE.stopAdvertise();
  }
}

bool isBleConnected() {
  return BLE.connected();
}
