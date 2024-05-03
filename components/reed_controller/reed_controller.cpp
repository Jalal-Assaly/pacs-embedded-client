#include "reed_controller.h"

#include <Arduino.h>

void reed_init(void (*interrupt_fn)())
{
    pinMode(REED_DIGITAL_INPUT, INPUT_PULLUP);
    attachInterrupt(REED_DIGITAL_INPUT, interrupt_fn, FALLING);
}