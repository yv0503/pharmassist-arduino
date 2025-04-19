#include "rtc_handler.h"

RTCHandler::RTCHandler(const uint8_t resetPin, const uint8_t clockPin, const uint8_t dataPin)
    : rtc(resetPin, clockPin, dataPin) {
}

void RTCHandler::initialize() {
    rtc.init();
    
    if (rtc.isHalted()) {
        Serial.println("RTC was not running, setting default time");
        Ds1302::DateTime dt = {
            .year = 25,
            .month = 1,
            .day = 1,
            .hour = 0,
            .minute = 0,
            .second = 0,
            .dow = 3
        };
        rtc.setDateTime(&dt);
    } else {
        Serial.println("RTC is running properly");
    }
}

bool RTCHandler::isRunning() {
    return !rtc.isHalted();
}

String RTCHandler::getFormattedTime() {
    const Ds1302::DateTime now = getCurrentDateTime();
    
    String timeStr = "";
    
    if (now.hour < 10) timeStr += "0";
    timeStr += String(now.hour) + ":";
    
    if (now.minute < 10) timeStr += "0";
    timeStr += String(now.minute) + ":";
    
    if (now.second < 10) timeStr += "0";
    timeStr += String(now.second);
    
    return timeStr;
}

String RTCHandler::getFormattedDate() {
    const Ds1302::DateTime now = getCurrentDateTime();
    
    String dateStr = "";
    
    if (now.day < 10) dateStr += "0";
    dateStr += String(now.day) + "/";
    
    if (now.month < 10) dateStr += "0";
    dateStr += String(now.month) + "/";
    
    dateStr += "20" + String(now.year);  // Assuming 20xx years
    
    return dateStr;
}

String RTCHandler::getFormattedDateTime() {
    return getFormattedDate() + " " + getFormattedTime();
}

void RTCHandler::setTimeFromEpoch(const unsigned long epochSeconds) {
    // Unix timestamp starts from January 1, 1970.
    // Calculate year (including leap years)
    constexpr unsigned long secondsPerDay = 86400;
    constexpr unsigned long secondsPerYear = 365 * secondsPerDay;
    constexpr unsigned long secondsPerLeapYear = 366 * secondsPerDay;
    
    int year = 1970;
    unsigned long remainingSeconds = epochSeconds;
    
    while (true) {
        const bool isLeapYear = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
        const unsigned long yearSeconds = isLeapYear ? secondsPerLeapYear : secondsPerYear;
        
        if (remainingSeconds < yearSeconds) {
            break;
        }
        
        remainingSeconds -= yearSeconds;
        year++;
    }
    
    // Calculate month
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        daysInMonth[1] = 29; // February has 29 days in leap years
    }
    
    int month = 0;
    unsigned long days = remainingSeconds / secondsPerDay;
    remainingSeconds %= secondsPerDay;
    
    while (days >= daysInMonth[month]) {
        days -= daysInMonth[month];
        month++;
    }
    
    // Calculate day, hour, minute, second
    const int day = days + 1; // Days are 1-based
    const int hour = remainingSeconds / 3600;
    remainingSeconds %= 3600;
    const int minute = remainingSeconds / 60;
    const int second = remainingSeconds % 60;
    
    // Calculate day of the week (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    // January 1, 1970, was a Thursday (4)
    const unsigned long totalDays = epochSeconds / secondsPerDay;
    int dow = (totalDays + 4) % 7;
    // Convert to DS1302 format where 1 = Monday, ..., 7 = Sunday
    dow = dow == 0 ? 7 : dow;
    
    // Set the RTC
    Ds1302::DateTime dt = {
        .year = static_cast<uint8_t>(year % 100), // 2-digit year
        .month = static_cast<uint8_t>(month + 1), // 1-based month
        .day = static_cast<uint8_t>(day),
        .hour = static_cast<uint8_t>(hour),
        .minute = static_cast<uint8_t>(minute),
        .second = static_cast<uint8_t>(second),
        .dow = static_cast<uint8_t>(dow)
    };
    
    rtc.setDateTime(&dt);
}

unsigned long RTCHandler::getEpochTime() {
    const Ds1302::DateTime now = getCurrentDateTime();
    
    // Convert to Unix timestamp
    // This is a simplified calculation that doesn't account for all leap years correctly
    const int year = 2000 + now.year; // Assuming 20xx years
    
    // Count seconds for years since 1970
    unsigned long seconds = 0;
    for (int y = 1970; y < year; y++) {
        if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) {
            seconds += 366 * 24 * 60 * 60; // Leap year
        } else {
            seconds += 365 * 24 * 60 * 60; // Regular year
        }
    }
    
    // Add seconds for the months in the current year
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        daysInMonth[1] = 29; // February has 29 days in leap years
    }
    
    for (int m = 0; m < now.month - 1; m++) {
        seconds += daysInMonth[m] * 24 * 60 * 60;
    }
    
    // Add seconds for days, hours, minutes, seconds
    seconds += (now.day - 1) * 24 * 60 * 60;
    seconds += now.hour * 60 * 60;
    seconds += now.minute * 60;
    seconds += now.second;
    
    return seconds;
}

Ds1302::DateTime RTCHandler::getCurrentDateTime() {
    Ds1302::DateTime now;
    rtc.getDateTime(&now);
    return now;
}
