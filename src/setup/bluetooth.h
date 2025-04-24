#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_LED_Matrix.h>

void setupBluetooth(String &ssid, String &password, LiquidCrystal_I2C &lcd);
void broadcastWiFiStatus(int status, const String &message, LiquidCrystal_I2C &lcd);
void bluetoothLoop();
void closeBluetooth();
void startBluetooth(String &ssid, String &password, LiquidCrystal_I2C &lcd);
void stopBluetooth();
void checkBluetoothTimeout();
void resetBluetoothTimeout();
bool isBleConnected();
bool isBluetoothActive();
void restartBleAdvertising(LiquidCrystal_I2C &lcd);
void runBluetoothSetup(String &ssid, String &password, ArduinoLEDMatrix &matrix, LiquidCrystal_I2C &lcd);

#endif // BLUETOOTH_H
