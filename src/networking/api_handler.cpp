#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include <constants.h>
#include "api_handler.h"

#include "utils/string_hash.h"

ApiResponse ApiHandler::processRequest(const String& endpoint, const String& method, LiquidCrystal_I2C& lcd) {
  Serial.print("API Request: ");
  Serial.print(method);
  Serial.print(" ");
  Serial.println(endpoint);

  if (method == "GET") {
    switch (hash(endpoint.c_str())) {
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
      default:
        break;
    }
  }

  ApiResponse response;
  response.statusCode = 404;
  response.contentType = "application/json";

  JsonDocument doc;
  doc["error"] = "Endpoint not found";
  serializeJson(doc, response.body);
  
  return response;
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
  
  JsonDocument doc;
  doc["status"] = "success";
  doc["message"] = "Factory reset performed. Device restart required.";
  serializeJson(doc, response.body);
  
  return response;
}

ApiResponse ApiHandler::handleHelloWorld(LiquidCrystal_I2C& lcd) {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("    Hello World!");

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = "application/json";

  JsonDocument doc;
  doc["status"] = "success";
  doc["message"] = "Hello World from PharmAssist!";
  serializeJson(doc, response.body);

  return response;
}

ApiResponse ApiHandler::handleDisplayName(LiquidCrystal_I2C &lcd) {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("    PharmAssist");

  ApiResponse response;
  response.statusCode = 200;
  response.contentType = "application/json";

  JsonDocument doc;
  doc["status"] = "success";
  doc["message"] = "PharmAssist";
  serializeJson(doc, response.body);

  return response;
}

