#ifndef LED_CONTROLLER
#define LED_CONTROLLER

// Define the pins for RGB LED
#define     LED_RED       (4)
#define     LED_GREEN     (2)
#define     LED_BLUE      (15)

// Define colors
#define     LED_OFF     0
#define     LED_ON      255

void led_init();
void led_setRGB(int R, int G, int B);

#endif