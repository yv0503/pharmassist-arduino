#include <Arduino.h>
#include <ArduinoJson.h>

#include "api_handler.h"

#include "utils/lcd_handler.h"
#include "utils/preferences_handler.h"
#include "utils/rtc_handler.h"
#include "utils/string_hash.h"

ApiResponse ApiHandler::processRequest(
  const String &endpoint,
  const String &method,
  const String &requestBody,
  const PreferencesHandler &prefsHandler,
  bool &isDeviceAcknowledged,
  LCDHandler &lcdHandler,
  RTCHandler &rtcHandler
) {
  Serial.print(F("API Request: "));
  Serial.print(method);
  Serial.print(F(" "));
  Serial.println(endpoint);

  if (!requestBody.isEmpty()) {
    Serial.print(F("Request Body: "));
    Serial.println(requestBody);
  }

  if (method == F("GET")) {
    switch (hash(endpoint.c_str())) {
      case "/api/status_check"_hash:
        return handleStatusCheck(isDeviceAcknowledged);
      case "/api/hello_world"_hash:
        return handleHelloWorld(lcdHandler);
      default:
        break;
    }
  }

  if (method == F("POST")) {
    switch (hash(endpoint.c_str())) {
      case "/api/reset"_hash:
        return handleReset(lcdHandler, prefsHandler);
      case "/api/display_message"_hash:
        return handleDisplayMessage(lcdHandler, requestBody);
      case "/api/set_time"_hash:
        return handleSetTime(requestBody, rtcHandler);
      case "/api/acknowledge"_hash:
        return handleAcknowledge(isDeviceAcknowledged);
      default:
        break;
    }
  }

  ApiResponse response;
  response.statusCode = 404;
  response.contentType = F("application/json");

  JsonDocument message;
  message["error"] = F("Endpoint not found");
  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleAcknowledge(bool &isDeviceAcknowledged) {
  isDeviceAcknowledged = true;

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = F("application/json");

  JsonDocument message;
  message["status"] = F("success");
  message["message"] = F("Device acknowledged successfully.");
  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleStatusCheck(const bool &isDeviceAcknowledged) {
  ApiResponse response;
  response.statusCode = 200;
  response.contentType = F("application/json");

  JsonDocument message;

  if (isDeviceAcknowledged) {
    message["status"] = F("success");
    message["message"] = F("PharmAssist is running and ready to assist!");
  } else {
    message["status"] = F("needs_acknowledgement");
    message["message"] = F("PharmAssist is running but needs Bluetooth acknowledgment.");
  }

  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleReset(const LCDHandler &lcdHandler, const PreferencesHandler &prefsHandler) {
  prefsHandler.deletePreferences();
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println(F("Factory reset performed via API"));

  lcdHandler.clear();
  lcdHandler.displayMsgCentered(F("Factory reset done"), 1);
  lcdHandler.displayMsgCentered(F("Restart Required"), 2);

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = F("application/json");

  JsonDocument message;
  message["status"] = F("success");
  message["message"] = F("Factory reset performed. Device restart required.");
  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleHelloWorld(const LCDHandler &lcdHandler) {
  lcdHandler.clear();
  lcdHandler.displayMsgCentered(F("Hello World"), 1);

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = F("application/json");

  JsonDocument message;
  message["status"] = F("success");
  message["message"] = F("Hello World from PharmAssist!");
  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleDisplayMessage(LCDHandler &lcdHandler, const String &requestBodyStr) {
  ApiResponse response;
  response.contentType = F("application/json");

  JsonDocument requestBody;
  deserializeJson(requestBody, requestBodyStr);

  const String line1 = requestBody["line1"];
  const String line2 = requestBody["line2"];
  const String line3 = requestBody["line3"];
  const String line4 = requestBody["line4"];

  if (line1.length() > 20 || line2.length() > 20 || line3.length() > 20 || line4.length() > 20) {
    response.statusCode = 400;

    JsonDocument errorMessage;
    errorMessage["error"] = F("Message too long");
    errorMessage["message"] = F("Each line must be less than or equal to 20 characters.");
    serializeJson(errorMessage, response.body);
    return response;
  }

  lcdHandler.clear();
  lcdHandler.displayMsg(line1, 1);
  lcdHandler.displayMsg(line2, 2);
  lcdHandler.displayMsg(line3, 3);
  lcdHandler.displayMsg(line4, 4);

  response.statusCode = 200;
  JsonDocument message;
  message["status"] = F("success");
  message["message"] = F("Message displayed on LCD");
  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleSetTime(const String &requestBodyStr, RTCHandler &rtcHandler) {
  ApiResponse response;
  response.contentType = F("application/json");

  JsonDocument requestBody;
  DeserializationError error = deserializeJson(requestBody, requestBodyStr);

  if (error) {
    response.statusCode = 400;

    JsonDocument errorMessage;
    errorMessage["error"] = F("Invalid JSON");
    errorMessage["message"] = error.c_str();
    serializeJson(errorMessage, response.body);
    return response;
  }

  if (requestBody["epochTime"].isNull()) {
    response.statusCode = 400;

    JsonDocument errorMessage;
    errorMessage["error"] = F("Missing required field");
    errorMessage["message"] = F("The 'epochTime' field is required.");
    serializeJson(errorMessage, response.body);
    return response;
  }

  unsigned long epochTime = requestBody["epochTime"];

  rtcHandler.setTimeFromEpoch(epochTime);

  response.statusCode = 200;

  JsonDocument message;
  message["status"] = F("success");
  message["message"] = F("Time set successfully");
  message["currentTime"] = rtcHandler.getFormattedDateTime();
  message["epochTime"] = rtcHandler.getEpochTime();
  serializeJson(message, response.body);

  return response;
}
