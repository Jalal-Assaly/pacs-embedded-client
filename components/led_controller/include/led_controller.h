#ifndef LED_CONTROLLER
#define LED_CONTROLLER

// Define the pins for RGB LED
#define     LED_RED       (19)
#define     LED_GREEN     (18)
#define     LED_BLUE      (5)

// Define colors
#define     LED_OFF     0
#define     LED_ON      255

void led_init();
void led_setRGB(int R, int G, int B);

#endif