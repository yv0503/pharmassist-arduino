#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr int isInitializedAddress = 0;
constexpr int lastKnownIpStartAddress = 1;
constexpr int ssidStartAddress = 5;
constexpr int passwordStartAddress = 65;
constexpr int maxStrLength = 64;

const String WIFI_SERVICE_UUID = "580FDA20-7887-48E7-92E6-B1E14121A372";
const String WIFI_JSON_CHARACTERISTIC_UUID = "26BDA59C-433D-491A-9964-0C9E7E80FCDE";
const String WIFI_STATUS_CHARACTERISTIC_UUID = "74FD2D9A-4A07-4DD7-A5D3-52C439CCDF77";
const String WIFI_ACK_CHARACTERISTIC_UUID = "A894B9AD-19CB-47D3-B50D-60CE41720F00";

const String DEVICE_NAME = "PharmAssist";
constexpr unsigned long BLE_PAIRING_TIMEOUT_MS = 300000;
constexpr unsigned long BLE_INACTIVITY_TIMEOUT_MS = 180000;
constexpr int JSON_BUFFER_SIZE = 256;

#endif // CONSTANTS_H
