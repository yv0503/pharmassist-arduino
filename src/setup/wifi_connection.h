#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <LiquidCrystal_I2C.h>

int connectToWiFi(const String &ssid, const String &password, ArduinoLEDMatrix &matrix, LiquidCrystal_I2C &lcd);

#endif // WIFI_CONNECTION_H
