#ifndef PREFERENCES_HANDLER_H
#define PREFERENCES_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

class PreferencesHandler {
    String filename = "prefs.dat";

    bool readJsonFromFile(JsonDocument &doc) const;

    [[nodiscard]] bool writeJsonToFile(const JsonDocument &doc) const;

public:
    [[nodiscard]] bool preferencesExist() const;

    [[nodiscard]] String getString(const String &key, const String &defaultValue = "") const;

    [[nodiscard]] bool setString(const String &key, const String &value) const;

    [[nodiscard]] bool createOrUpdatePreferences(const std::map<String, String> &preferences) const;

    void deletePreferences() const;
};

#endif // PREFERENCES_HANDLER_H
