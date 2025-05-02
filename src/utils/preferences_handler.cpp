#include "preferences_handler.h"

#include <SD.h>

bool PreferencesHandler::preferencesExist() const {
    return SD.exists(filename);
}

bool PreferencesHandler::readJsonFromFile(JsonDocument &doc) const {
    if (!preferencesExist()) {
        return false;
    }

    File prefsFile = SD.open(filename, FILE_READ);
    if (!prefsFile) {
        return false;
    }

    const DeserializationError error = deserializeJson(doc, prefsFile);
    prefsFile.close();

    return !error;
}

bool PreferencesHandler::writeJsonToFile(const JsonDocument &doc) const {
    if (SD.exists(filename)) {
        SD.remove(filename);
    }

    if (File prefsFile = SD.open(filename, FILE_WRITE)) {
        String json = "";
        const bool success = serializeJson(doc, json) > 0;
        prefsFile.print(json);
        prefsFile.close();

        return success;
    }

    return false;
}

String PreferencesHandler::getString(const String &key, const String &defaultValue) const {
    JsonDocument doc;
    if (!readJsonFromFile(doc) || !doc[key].is<String>()) {
        return defaultValue;
    }
    return doc[key].as<String>();
}

bool PreferencesHandler::setString(const String &key, const String &value) const {
    JsonDocument doc;
    readJsonFromFile(doc);

    doc[key] = value;

    return writeJsonToFile(doc);
}

bool PreferencesHandler::createOrUpdatePreferences(const std::map<String, String> &preferences) const {
    JsonDocument doc;
    readJsonFromFile(doc);

    for (const auto &[key, value]: preferences) {
        doc[key] = value;
    }

    return writeJsonToFile(doc);
}

void PreferencesHandler::deletePreferences() const {
    if (SD.exists(filename)) {
        SD.remove(filename);
    }
}
