#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <Arduino_LED_Matrix.h>

void setupBluetooth(String& ssid, String& password);
void bluetoothLoop();
bool isBleConnected();
void runBluetoothSetup(String& ssid, String& password, ArduinoLEDMatrix& matrix);

#endif // BLUETOOTH_H
