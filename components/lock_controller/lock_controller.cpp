#include "lock_controller.h"

#include <Arduino.h>

void lock_init() {
    pinMode(LOCK, OUTPUT);
}

void lock_open() {
    digitalWrite(LOCK, HIGH);
}

void lock_close() {
    digitalWrite(LOCK, LOW);
}