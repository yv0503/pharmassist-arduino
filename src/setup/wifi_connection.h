#pragma once

#include <Arduino.h>
#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include "../led_matrix/wifi_matrix.h"

int connectToWiFi(const String& ssid, const String& password, ArduinoLEDMatrix& matrix);
