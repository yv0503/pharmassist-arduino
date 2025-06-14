#include "servo_handler.h"

ServoHandler::ServoHandler()
    {
    _servo = new Servo();
}

void ServoHandler::initialize(const uint8_t servo_pin) const {
    _servo->attach(servo_pin);
}

void ServoHandler::toNextContainer(const uint8_t timeInMillis, const uint8_t currentContainer,const uint8_t nextContainer) const {
    if (currentContainer != 4) {
        _servo->write(180);
        delay(timeInMillis * (nextContainer - currentContainer));
        _servo->write(90);
    }
    if (currentContainer == 4) {
        resetPosition(timeInMillis, currentContainer);
    }
}

void ServoHandler::resetPosition(const uint8_t timeInMillis, const uint8_t currentContainer) const {
    if (currentContainer != 0) {
        _servo->write(180);
        delay(timeInMillis * ( 5 - currentContainer));
        _servo->write(90);
    }

}



