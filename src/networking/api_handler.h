#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

#include "utils/lcd_handler.h"
#include "utils/preferences_handler.h"
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
    const PreferencesHandler &prefsHandler,
    bool &isDeviceAcknowledged,
    LCDHandler &lcdHandler, RTCHandler &rtcHandler
  );

private:
  static ApiResponse handleAcknowledge(bool &isDeviceAcknowledged);

  static ApiResponse handleStatusCheck(const bool &isDeviceAcknowledged);

  static ApiResponse handleReset(const LCDHandler &lcdHandler, const PreferencesHandler &prefsHandler);

  static ApiResponse handleHelloWorld(const LCDHandler &lcdHandler);

  static ApiResponse handleDisplayMessage(LCDHandler &lcdHandler, const String &requestBodyStr);

  static ApiResponse handleSetTime(const String &requestBodyStr, RTCHandler &rtcHandler);
};

#endif
