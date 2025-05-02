#include "lcd_handler.h"

LCDHandler::LCDHandler(const uint8_t lcd_addr, const uint8_t columns, const uint8_t rows) : _columns(columns),
    _rows(rows) {
    _lcd = new LiquidCrystal_I2C(lcd_addr, columns, rows);
}

LCDHandler::~LCDHandler() {
    delete _lcd;
}

void LCDHandler::initialize() const {
    _lcd->init();
    _lcd->backlight();
    clear();
}

void LCDHandler::clear() const {
    _lcd->clear();
}

void LCDHandler::clearLine(const uint8_t line) const {
    if (line >= _rows) return;
    _lcd->setCursor(0, line);
    for (uint8_t i = 0; i < _columns; i++) {
        _lcd->print(" ");
    }
}

void LCDHandler::displayTitle(const String &title) const {
    displayMsgCentered(title, 0);
}

void LCDHandler::displayTime(const String &time) const {
    displayMsgCentered(time, 3);
}

void LCDHandler::displayDate(const String &weekday, const String &date) const {
    const String fullDate = weekday + ", " + date;
    displayMsgCentered(fullDate, 2);
}

void LCDHandler::displayMsg(const String &message, const uint8_t line) const {
    if (line >= _rows) return;
    _lcd->setCursor(0, line);
    _lcd->print(message);
}

void LCDHandler::displayMsgCentered(const String &text, const uint8_t line) const {
    if (line >= _rows) return;

    int8_t padding = (_columns - text.length()) / 2;
    if (padding < 0) padding = 0;

    _lcd->setCursor(0, line);
    for (uint8_t i = 0; i < padding; i++) {
        _lcd->print(" ");
    }
    _lcd->print(text);

    const int8_t remaining = _columns - padding - text.length();
    for (int8_t i = 0; i < remaining; i++) {
        _lcd->print(" ");
    }
}
