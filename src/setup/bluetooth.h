#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <LiquidCrystal_I2C.h>

void setupBluetooth(String& ssid, String& password);
void bluetoothLoop();
bool isBleConnected();
void runBluetoothSetup(String& ssid, String& password, ArduinoLEDMatrix& matrix, LiquidCrystal_I2C& lcd);

#endif // BLUETOOTH_H
