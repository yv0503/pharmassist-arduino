#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <BLECharacteristic.h>
#include <ArduinoJson.h>
#include <Arduino_LED_Matrix.h>

#include <constants.h>
#include "../led_matrix/bluetooth_matrix.h"
#include "bluetooth.h"

#include <WiFi.h>
#include <WiFiTypes.h>

#include "wifi_credentials.h"
#include "utils/ip_handler.h"

BLEService bluetoothService(WIFI_SERVICE_UUID.c_str());
BLEStringCharacteristic wifiJsonCharacteristic(WIFI_JSON_CHARACTERISTIC_UUID.c_str(), BLEWrite | BLERead,
                                               JSON_BUFFER_SIZE);
BLEStringCharacteristic wifiStatusCharacteristic(WIFI_STATUS_CHARACTERISTIC_UUID.c_str(), BLERead | BLENotify,
                                                JSON_BUFFER_SIZE);
BLEStringCharacteristic wifiAckCharacteristic(WIFI_ACK_CHARACTERISTIC_UUID.c_str(), BLEWrite | BLERead,
                                             JSON_BUFFER_SIZE);

String *ssidPtr = nullptr;
String *passwordPtr = nullptr;
bool setupComplete = false;
extern bool isDeviceAcknowledged;
bool bluetoothActive = false;
unsigned long lastBluetoothActivityTime = 0;

// ReSharper disable CppParameterNeverUsed
void onJsonReceived(BLEDevice central, BLECharacteristic characteristic) { // NOLINT(*-unnecessary-value-param)
  resetBluetoothTimeout();
  String jsonStr = wifiJsonCharacteristic.value();
  Serial.print(F("Received JSON: "));
  Serial.println(jsonStr);

  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, jsonStr);

  if (error) {
    Serial.print(F("JSON parsing failed: "));
    Serial.println(error.c_str());
    return;
  }

  if (!doc["ssid"].isNull() && !doc["password"].isNull()) {
    *ssidPtr = doc["ssid"].as<String>();
    *passwordPtr = doc["password"].as<String>();

    Serial.print(F("SSID received: "));
    Serial.println(*ssidPtr);
    Serial.print(F("Password length: "));
    Serial.println(passwordPtr->length());

    if (ssidPtr->length() > 0 && passwordPtr->length() > 0) {
      saveWiFiCredentials(*ssidPtr, *passwordPtr);
      EEPROM.update(isInitializedAddress, 1);
      setupComplete = true;

      JsonDocument response;
      response["status"] = "success";
      String responseStr;
      serializeJson(response, responseStr);
      wifiJsonCharacteristic.writeValue(responseStr);

      Serial.println(F("Credentials saved, BLE setup complete"));
    } else {
      Serial.println(F("Error: SSID or password is empty"));
    }
  } else {
    Serial.println(F("Error: JSON missing required fields"));
  }
}

void onAckReceived(BLEDevice central, BLECharacteristic characteristic) { // NOLINT(*-unnecessary-value-param)
  resetBluetoothTimeout();
  String ackStr = wifiAckCharacteristic.value();
  Serial.print(F("Received Acknowledgment: "));
  Serial.println(ackStr);

  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, ackStr);

  if (error) {
    Serial.print(F("JSON parsing failed: "));
    Serial.println(error.c_str());
    return;
  }

  if (!doc["acknowledged"].isNull() && doc["acknowledged"].as<bool>()) {
    isDeviceAcknowledged = true;
    Serial.println(F("Device acknowledged WiFi status"));
  }
}

// ReSharper disable once CppPassValueParameterByConstReference
void onBLEConnected(BLEDevice central) { // NOLINT(*-unnecessary-value-param)
  resetBluetoothTimeout();
  Serial.print(F("Connected to central: "));
  Serial.println(central.address());
}

// ReSharper disable once CppPassValueParameterByConstReference
void onBLEDisconnected(BLEDevice central) { // NOLINT(*-unnecessary-value-param)
  Serial.print(F("Disconnected from central: "));
  Serial.println(central.address());

  if (!setupComplete) {
    Serial.println(F("Setup was not completed. Restarting advertising..."));
    BLE.advertise();
    resetBluetoothTimeout();
  }
}

void resetBluetoothTimeout() {
  lastBluetoothActivityTime = millis();
  Serial.println(F("Bluetooth timeout reset"));
}

void checkBluetoothTimeout() {
  if (!bluetoothActive || !isDeviceAcknowledged || isBleConnected())
    return;
    
  if (millis() - lastBluetoothActivityTime > BLE_INACTIVITY_TIMEOUT_MS) {
    Serial.println(F("Bluetooth inactivity timeout reached"));
    stopBluetooth();
  }
}

void setupBluetooth(String &ssid, String &password, LiquidCrystal_I2C &lcd) {
  ssidPtr = &ssid;
  passwordPtr = &password;
  setupComplete = false;

  startBluetooth(ssid, password, lcd);
}

void startBluetooth(String &ssid, String &password, LiquidCrystal_I2C &lcd) {
  if (bluetoothActive) {
    resetBluetoothTimeout();
    return;
  }
  
  ssidPtr = &ssid;
  passwordPtr = &password;
  
  Serial.println(F("Starting Bluetooth..."));
  
  if (!BLE.begin()) {
    Serial.println(F("Starting Bluetooth® Low Energy module failed!"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Bluetooth init failed"));
    lcd.setCursor(0, 2);
    lcd.print(F("Restart your device."));

    // ReSharper disable once CppDFAEndlessLoop
    while (true);
  }

  BLE.setDeviceName(DEVICE_NAME.c_str());
  BLE.setLocalName(DEVICE_NAME.c_str());
  BLE.setAdvertisedService(bluetoothService);

  bluetoothService.addCharacteristic(wifiJsonCharacteristic);
  bluetoothService.addCharacteristic(wifiStatusCharacteristic);
  bluetoothService.addCharacteristic(wifiAckCharacteristic);

  wifiJsonCharacteristic.setEventHandler(BLEWritten, onJsonReceived);
  wifiAckCharacteristic.setEventHandler(BLEWritten, onAckReceived);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
  BLE.setEventHandler(BLEConnected, onBLEConnected);

  BLE.addService(bluetoothService);
  BLE.advertise();
  bluetoothActive = true;
  resetBluetoothTimeout();

  Serial.println(F("Bluetooth® device active, waiting for connections..."));
}

void stopBluetooth() {
  if (bluetoothActive) {
    BLE.stopAdvertise();
    BLE.end();
    bluetoothActive = false;
    Serial.println(F("Bluetooth service completely stopped"));
  }
}

void broadcastWiFiStatus(const int status, const String &message, LiquidCrystal_I2C &lcd) {
  if (!bluetoothActive && ssidPtr != nullptr) {
    startBluetooth(*ssidPtr, *passwordPtr, lcd);
  }

  int statusCode = 0;
  const IPAddress localIp = WiFi.localIP();

  if (status == WL_CONNECTED) {
    statusCode = isSameIp(loadLastKnownIp(), localIp) ? 200 : 300;
  } else {
    statusCode = 400;
  }

  resetBluetoothTimeout();
  JsonDocument doc;
  doc["status"] = statusCode;
  doc["message"] = message;
  doc["ip"] = localIp.toString();
  
  String statusStr;
  serializeJson(doc, statusStr);
  
  wifiStatusCharacteristic.writeValue(statusStr);
  Serial.print(F("Broadcasting WiFi status: "));
  Serial.println(statusStr);
}

void bluetoothLoop() {
  if (bluetoothActive) {
    BLE.poll();
    checkBluetoothTimeout();
  }
}

void closeBluetooth() {
  if (bluetoothActive) {
    BLE.stopAdvertise();
    bluetoothActive = false;
    Serial.println(F("Bluetooth advertising stopped but connection kept active"));
  }
}

bool isBleConnected() {
  return BLE.connected();
}

bool isBluetoothActive() {
  return bluetoothActive;
}

void restartBleAdvertising(LiquidCrystal_I2C &lcd) {
  if (!bluetoothActive && ssidPtr != nullptr) {
    startBluetooth(*ssidPtr, *passwordPtr, lcd);
    return;
  }
  
  if (!BLE.advertise() && bluetoothActive) {
    BLE.advertise();
    resetBluetoothTimeout();
    Serial.println(F("Restarted Bluetooth advertising"));
  }
}

void runBluetoothSetup(String &ssid, String &password, ArduinoLEDMatrix &matrix, LiquidCrystal_I2C &lcd) {
  Serial.println(F("Starting Bluetooth setup..."));
  setupBluetooth(ssid, password, lcd);

  matrix.begin();
  matrix.loadFrame(bluetooth_matrix[0]);
  
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(F("Wi-Fi Setup Mode"));
  lcd.setCursor(1, 1);
  lcd.print(F("Connect to device:"));
  lcd.setCursor(4, 2);
  lcd.print(DEVICE_NAME);
  lcd.setCursor(3, 3);
  lcd.print(F("via Bluetooth"));

  bool wasConnected = false;
  unsigned long previousMillis = 0;
  byte currentFrame = 0;

  while (EEPROM.read(isInitializedAddress) != 1) {
    constexpr long interval = 1000;
    bluetoothLoop();

    const bool isConnected = isBleConnected();
    const unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      if (!isConnected) {
        currentFrame = 1 - currentFrame;
        matrix.loadFrame(bluetooth_matrix[currentFrame]);
        
        lcd.setCursor(3, 3);
        lcd.print(currentFrame == 0 ? F("  Waiting...      ") : F("via Bluetooth     "));
      } else if (!wasConnected) {
        matrix.loadFrame(bluetooth_matrix[1]);
        lcd.setCursor(0, 2);
        lcd.print(F("  Device connected  "));
        lcd.setCursor(0, 3);
        lcd.print(F(" Waiting for config"));
        wasConnected = true;
      }
    }

    if (!isConnected && wasConnected) {
      wasConnected = false;
      lcd.setCursor(0, 2);
      lcd.print("    " + DEVICE_NAME + "    ");
      lcd.setCursor(0, 3);
      lcd.print(F("   Disconnected    "));
      delay(1000);
      lcd.setCursor(0, 3);
      lcd.print(F("   via Bluetooth    "));
    }
  }

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Wi-Fi Setup Complete"));
  lcd.setCursor(0, 2);
  lcd.print(F("Connecting to WiFi..."));
}
