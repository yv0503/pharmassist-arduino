#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>

#include "utils/lcd_handler.h"
#include "utils/preferences_handler.h"

void runBluetoothSetup(String &ssid, String &password, PreferencesHandler &prefsHandler, const LCDHandler &lcdHandler);

void startBluetooth(String &ssid, String &password, const LCDHandler &lcdHandler, PreferencesHandler &prefsHandler);

void stopBluetooth();

void resetBluetoothTimeout();

bool isBluetoothTimeoutElapsed();

void onBLEDisconnected(BLEDevice device);

void onBLEConnected(BLEDevice device);

void onJsonReceived(BLEDevice device, BLECharacteristic characteristic);

void onAckReceived(BLEDevice device, BLECharacteristic characteristic);

void bluetoothLoop();

bool isBleConnected();

bool isBleActive();

void broadcastWiFiStatus(int status, const String &message);

#endif //BLUETOOTH_H
