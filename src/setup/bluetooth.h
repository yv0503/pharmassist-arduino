#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>

void setupBluetooth(String& ssid, String& password);
void bluetoothLoop();
bool isBleConnected();

#endif // BLUETOOTH_H
