#pragma once

#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

#include "../led_matrix/wifi_matrix.h"

int connectToWiFi(const String &ssid, const String &password, ArduinoLEDMatrix &matrix, LiquidCrystal_I2C &lcd);
