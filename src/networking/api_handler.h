#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

#include "utils/rtc_handler.h"

struct ApiResponse {
  int statusCode;
  String contentType;
  String body;
};

class ApiHandler {
public:
  static ApiResponse processRequest(
    const String &endpoint,
    const String &method,
    const String &requestBody,
    bool &isDeviceAcknowledged,
    LiquidCrystal_I2C &lcd,
    RTCHandler &rtcHandler
  );

private:
  static ApiResponse handleAcknowledge(bool &isDeviceAcknowledged);
  static ApiResponse handleStatusCheck(const bool &isDeviceAcknowledged);
  static ApiResponse handleReset(LiquidCrystal_I2C& lcd);
  static ApiResponse handleHelloWorld(LiquidCrystal_I2C& lcd);
  static ApiResponse handleDisplayName(LiquidCrystal_I2C& lcd);
  static ApiResponse handleDisplayMessage(LiquidCrystal_I2C& lcd, const String& requestBodyStr);
  static ApiResponse handleSetTime(const String &requestBodyStr, RTCHandler &rtcHandler);
};

#endif
