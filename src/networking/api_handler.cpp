#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include <constants.h>
#include "api_handler.h"

#include "utils/string_hash.h"

ApiResponse ApiHandler::processRequest(const String& endpoint, const String& method, const String& requestBody, LiquidCrystal_I2C& lcd) {
  Serial.print("API Request: ");
  Serial.print(method);
  Serial.print(" ");
  Serial.println(endpoint);
  
  if (!requestBody.isEmpty()) {
    Serial.print("Request Body: ");
    Serial.println(requestBody);
  }

  if (method == "GET") {
    switch (hash(endpoint.c_str())) {
      case "/api/status_check"_hash:
        return handleStatusCheck();
      case "/api/hello_world"_hash:
        return handleHelloWorld(lcd);
      case "/api/display_name"_hash:
        return handleDisplayName(lcd);
      default:
        break;
    }
  }

  if (method == "POST") {
    switch (hash(endpoint.c_str())) {
      case "/api/reset"_hash:
        return handleReset(lcd);
      case "/api/display_message"_hash:
        return handleDisplayMessage(lcd, requestBody);
      default:
        break;
    }
  }

  ApiResponse response;
  response.statusCode = 404;
  response.contentType = "application/json";

  JsonDocument message;
  message["error"] = "Endpoint not found";
  serializeJson(message, response.body);
  
  return response;
}

ApiResponse ApiHandler::handleStatusCheck() {
  ApiResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.body = "PharmAssist is running and ready to assist!";

  return  response;
}

ApiResponse ApiHandler::handleReset(LiquidCrystal_I2C& lcd) {
  EEPROM.update(isInitializedAddress, 0);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("Factory reset performed via API");

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(" Factory reset done");
  lcd.setCursor(0, 2);
  lcd.print("  Restart Required");

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = "application/json";
  
  JsonDocument message;
  message["status"] = "success";
  message["message"] = "Factory reset performed. Device restart required.";
  serializeJson(message, response.body);
  
  return response;
}

ApiResponse ApiHandler::handleHelloWorld(LiquidCrystal_I2C& lcd) {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("    Hello World!");

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = "application/json";

  JsonDocument message;
  message["status"] = "success";
  message["message"] = "Hello World from PharmAssist!";
  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleDisplayName(LiquidCrystal_I2C &lcd) {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("    PharmAssist");

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = "application/json";

  JsonDocument message;
  message["status"] = "success";
  message["message"] = "PharmAssist";
  serializeJson(message, response.body);

  return response;
}

ApiResponse ApiHandler::handleDisplayMessage(LiquidCrystal_I2C &lcd, const String &requestBodyStr) {
  ApiResponse response;
  response.contentType = "application/json";

  JsonDocument requestBody;
  deserializeJson(requestBody, requestBodyStr);

  const String line1 = requestBody["line1"];
  const String line2 = requestBody["line2"];
  const String line3 = requestBody["line3"];
  const String line4 = requestBody["line4"];

  if (line1.length() > 20 || line2.length() > 20 || line3.length() > 20 || line4.length() > 20) {
    response.statusCode = 400;

    JsonDocument errorMessage;
    errorMessage["error"] = "Message too long";
    errorMessage["message"] = "Each line must be less than or equal to 20 characters.";
    serializeJson(errorMessage, response.body);
    return response;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcd.setCursor(0, 2);
  lcd.print(line3);
  lcd.setCursor(0, 3);
  lcd.print(line4);
  
  response.statusCode = 200;
  JsonDocument message;
  message["status"] = "success";
  message["message"] = "Message displayed on LCD";
  serializeJson(message, response.body);
  
  return response;
}

