#include "buzzer_controller.h"

#include <Arduino.h>

void buzzer_init() {
    pinMode(BUZZER, OUTPUT);
}

void buzzer_on() {
    digitalWrite(BUZZER, HIGH);
}
void buzzer_off() {
    digitalWrite(BUZZER, LOW);
}

void buzzer_onFor(unsigned long duration_ms) {
    digitalWrite(BUZZER, HIGH);
    delay(duration_ms);
    digitalWrite(BUZZER, LOW);
    delay(duration_ms);
}