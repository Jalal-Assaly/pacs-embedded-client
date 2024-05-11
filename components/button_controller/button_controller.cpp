#include "button_controller.h"

#include <Arduino.h>

void button_init(void (*interrupt_fn)())
{
    pinMode(BUTTON_INPUT, INPUT);
    attachInterrupt(BUTTON_INPUT, interrupt_fn, RISING);
}