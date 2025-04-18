#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <LiquidCrystal_I2C.h>

void setupBluetooth(String& ssid, String& password, LiquidCrystal_I2C& lcd);
void bluetoothLoop();
bool isBleConnected();
void runBluetoothSetup(String& ssid, String& password, ArduinoLEDMatrix& matrix, LiquidCrystal_I2C& lcd);
void broadcastWiFiStatus(int status, const String& message, const String& ipAddress);
void closeBluetooth();

#endif // BLUETOOTH_H
