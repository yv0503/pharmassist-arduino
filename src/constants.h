#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr int isInitializedAddress = 0;
constexpr int ssidStartAddress = 1;
constexpr int passwordStartAddress = 65;
constexpr int maxStrLength = 64;

const String WIFI_SERVICE_UUID = "580FDA20-7887-48E7-92E6-B1E14121A372";
const String WIFI_JSON_CHARACTERISTIC_UUID = "26BDA59C-433D-491A-9964-0C9E7E80FCDE";

const String DEVICE_NAME = "PharmAssist";
constexpr unsigned long BLE_PAIRING_TIMEOUT_MS = 300000; // 5 minutes
constexpr int JSON_BUFFER_SIZE = 256;

#endif // CONSTANTS_H
