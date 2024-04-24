#include "led_controller.h"

#include <Arduino.h>

void led_init()
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
}

void led_setRGB(int R, int G, int B)
{
    analogWrite(LED_RED, R);
    analogWrite(LED_GREEN, G);
    analogWrite(LED_BLUE, B);
}