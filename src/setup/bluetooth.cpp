#include "bluetooth.h"

#include <ArduinoBLE.h>
#include <ArduinoJson.h>
#include <BLEService.h>
#include <BLEStringCharacteristic.h>

#include <constants.h>
#include <WiFi.h>

#include "wifi_connection.h"

BLEService bluetoothService(WIFI_SERVICE_UUID.c_str());
BLEStringCharacteristic wifiJsonChar(WIFI_JSON_CHARACTERISTIC_UUID.c_str(), BLEWrite | BLERead, JSON_BUFFER_SIZE);
BLEStringCharacteristic wifiStatusChar(WIFI_STATUS_CHARACTERISTIC_UUID.c_str(), BLERead | BLENotify, JSON_BUFFER_SIZE);
BLEStringCharacteristic wifiAckChar(WIFI_ACK_CHARACTERISTIC_UUID.c_str(), BLEWrite | BLERead, JSON_BUFFER_SIZE);

String *ssidPtr = nullptr;
String *passwordPtr = nullptr;
extern String lastIpKnown;
extern bool isInitialized;
extern bool isDeviceAcknowledged;
bool setupComplete = false;
bool bluetoothActive = false;
PreferencesHandler *prefsHandlerPtr = nullptr;

unsigned long bluetoothStartTime = 0;
constexpr unsigned long BLUETOOTH_TIMEOUT = 5 * 60 * 1000;

void runBluetoothSetup(String &ssid, String &password, PreferencesHandler &prefsHandler, const LCDHandler &lcdHandler) {
    startBluetooth(ssid, password, lcdHandler, prefsHandler);

    lcdHandler.clear();
    lcdHandler.displayMsgCentered(F("WiFi Setup Mode"), 0);
    lcdHandler.displayMsgCentered(F("Connect to device"), 1);
    lcdHandler.displayMsgCentered(DEVICE_NAME, 2);
    lcdHandler.displayMsgCentered(F("via Bluetooth"), 3);

    bool wasConnected = false;
    unsigned long previousMillis = 0;
    bool showWaiting = true;

    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (!isInitialized) {
        bluetoothLoop();

        const bool isConnected = isBleConnected();

        if (const unsigned long currentMillis = millis(); currentMillis - previousMillis >= 1000) {
            previousMillis = currentMillis;

            if (!isConnected) {
                showWaiting = !showWaiting;
                lcdHandler.displayMsgCentered(showWaiting ? "Waiting..." : "via Bluetooth", 3);
            } else if (!wasConnected) {
                lcdHandler.clearLine(1);
                lcdHandler.displayMsgCentered(F("Device connected"), 2);
                lcdHandler.displayMsgCentered(F("Waiting for config"), 3);
                wasConnected = true;
            }
        }

        if (!isConnected && wasConnected) {
            wasConnected = false;
            lcdHandler.displayMsgCentered(F("Connect to device"), 1);
            lcdHandler.displayMsgCentered("    " + DEVICE_NAME + "    ", 2);
            lcdHandler.displayMsgCentered(F("Disconnected"), 3);
            delay(1000);
            lcdHandler.displayMsgCentered(F("via Bluetooth"), 3);
        }

        if (isBluetoothTimeoutElapsed()) {
            stopBluetooth();
            break;
        }
    }

    lcdHandler.clear();
    lcdHandler.displayMsgCentered(F("Wi-Fi Setup Complete"), 1);
    lcdHandler.displayMsgCentered(F("Connecting to WiFi..."), 2);
}

void startBluetooth(String &ssid, String &password, const LCDHandler &lcdHandler, PreferencesHandler &prefsHandler) {
    ssidPtr = &ssid;
    passwordPtr = &password;

    if (bluetoothActive) {
        BLE.advertise();
        resetBluetoothTimeout();
        return;
    }

    prefsHandlerPtr = &prefsHandler;
    lcdHandler.clear();
    lcdHandler.displayMsgCentered(F("Starting Bluetooth.."), 1);

    if (!BLE.begin()) {
        Serial.println(F("Starting Bluetooth® Low Energy module failed!"));
        lcdHandler.clear();
        lcdHandler.displayMsgCentered(F("Bluetooth init failed"), 1);
        lcdHandler.displayMsgCentered(F("Restart your device"), 2);

        // ReSharper disable once CppDFAEndlessLoop
        while (true);
    }

    BLE.setDeviceName(DEVICE_NAME.c_str());
    BLE.setLocalName(DEVICE_NAME.c_str());
    BLE.setAdvertisedService(bluetoothService);

    bluetoothService.addCharacteristic(wifiJsonChar);
    bluetoothService.addCharacteristic(wifiStatusChar);
    bluetoothService.addCharacteristic(wifiAckChar);

    wifiJsonChar.setEventHandler(BLEWritten, onJsonReceived);
    wifiAckChar.setEventHandler(BLEWritten, onAckReceived);
    BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
    BLE.setEventHandler(BLEConnected, onBLEConnected);

    BLE.addService(bluetoothService);
    BLE.advertise();
    bluetoothActive = true;
    resetBluetoothTimeout();
    Serial.println(F("Bluetooth® device active. Waiting for connections..."));
}

void stopBluetooth() {
    if (bluetoothActive) {
        Serial.println(F("Stopping Bluetooth to save power"));
        BLE.disconnect();
        BLE.stopAdvertise();
        BLE.end();
        bluetoothActive = false;
    }
}

void resetBluetoothTimeout() {
    bluetoothStartTime = millis();
    Serial.println(F("Bluetooth timeout reset"));
}

bool isBluetoothTimeoutElapsed() {
    if (isBleConnected()) {
        resetBluetoothTimeout();
        return false;
    }

    return bluetoothActive &&
           millis() - bluetoothStartTime > BLUETOOTH_TIMEOUT &&
           isDeviceAcknowledged &&
           setupComplete;
}

void bluetoothLoop() {
    if (bluetoothActive)
        BLE.poll();
}

bool isBleConnected() {
    return BLE.connected();
}

bool isBleActive() {
    return bluetoothActive;
}

void broadcastWiFiStatus(const int status, const String &message) {
    int statusCode = 0;
    const IPAddress localIp = WiFi.localIP();

    if (status == WL_CONNECTED) {
        statusCode = isSameIp(IPAddress(lastIpKnown.c_str()), localIp) ? 200 : 300;
    } else {
        statusCode = 400;
    }

    JsonDocument doc;
    doc["status"] = statusCode;
    doc["message"] = message;
    doc["ip"] = localIp.toString();

    String statusStr;
    serializeJson(doc, statusStr);

    wifiStatusChar.writeValue(statusStr);
    Serial.print(F("Broadcasting WiFi status: "));
    Serial.println(statusStr);
}

// ReSharper disable CppPassValueParameterByConstReference, CppParameterNeverUsed

void onBLEConnected(BLEDevice device) { // NOLINT(*-unnecessary-value-param)
    Serial.print(F("Connected to device: "));
    Serial.println(device.address());
    resetBluetoothTimeout();
}

void onBLEDisconnected(BLEDevice device) { // NOLINT(*-unnecessary-value-param)
    Serial.print(F("Disconnected from device: "));
    Serial.println(device.address());

    if (!setupComplete) {
        Serial.println(F("Setup was not completed. Restarting advertising..."));
        BLE.advertise();
        resetBluetoothTimeout();
    }
}

void onJsonReceived(BLEDevice device, BLECharacteristic characteristic) { // NOLINT(*-unnecessary-value-param)
    String jsonStr;
    const int length = characteristic.valueLength();
    const uint8_t *val = characteristic.value();

    jsonStr.reserve(length);

    for (int i = 0; i < length; i++) {
        jsonStr += static_cast<char>(val[i]);
    }

    Serial.print(F("Received JSON: "));
    Serial.println(jsonStr);

    JsonDocument doc;
    JsonDocument response;
    String responseStr;

    auto sendResponse = [&](const char *status) {
        response["status"] = status;
        serializeJson(response, responseStr);
        characteristic.setValue(responseStr.c_str());
        Serial.print(F("Response sent: "));
        Serial.println(status);
    };

    if (deserializeJson(doc, jsonStr) != DeserializationError::Ok) {
        Serial.println(F("JSON parsing failed"));
        sendResponse("[error] parse_failed");
        return;
    }

    if (doc["ssid"].isNull() || doc["password"].isNull()) {
        Serial.println(F("Error: JSON missing required fields"));
        sendResponse("[error] missing_fields");
        return;
    }

    *ssidPtr = doc["ssid"].as<String>();
    *passwordPtr = doc["password"].as<String>();

    Serial.print(F("SSID received: "));
    Serial.println(*ssidPtr);
    Serial.print(F("Password length: "));
    Serial.println(passwordPtr->length());

    if (ssidPtr->isEmpty() || passwordPtr->length() < 8) {
        Serial.println(F("Error: SSID or password has invalid length"));
        sendResponse("[error] invalid_length");
        return;
    }

    if (!prefsHandlerPtr->setString(PREF_WIFI_SSID, *ssidPtr) ||
        !prefsHandlerPtr->setString(PREF_WIFI_PASSWORD, *passwordPtr)) {
        Serial.println(F("Failed to save WiFi credentials to preferences"));
        sendResponse("[error] save_failed");
        return;
    }

    setupComplete = true;
    isInitialized = true;
    Serial.println(F("Credentials saved, BLE setup complete"));
    sendResponse("success");
}

void onAckReceived(BLEDevice device, BLECharacteristic characteristic) { // NOLINT(*-unnecessary-value-param)
    String ackStr;
    const int length = characteristic.valueLength();
    const uint8_t *val = characteristic.value();

    ackStr.reserve(length);

    for (int i = 0; i < length; i++) {
        ackStr += static_cast<char>(val[i]);
    }
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
        resetBluetoothTimeout();
    }
}

