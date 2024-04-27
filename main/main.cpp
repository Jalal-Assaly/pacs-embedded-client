#include <Arduino.h>

#include <led_controller.h>
#include <buzzer_controller.h>
#include <lock_controller.h>
#include <nvs_attributes_manager.h>
#include <pn532_controller.h>
#include <lcd_controller.h>

extern "C"
{
#include <coap_client.h>

#include "esp_wifi.h"
#include "esp_event.h"

#include "protocol_examples_common.h"
}

typedef struct {
    uint8_t* payload;
} CoapTaskParams;

static char *byteArrayToCharArray(uint8_t *byteArray);

extern "C" void app_main(void)
{
    /* Initialize Arduino as a component */
    initArduino();
    Serial.begin(115200);

    /* Initialize LCD Screen */
    lcd_init();
    lcd_printHome("Device");
    lcd_printCustom("Initialization", 0, 2);

    /* Initialize LED RGB */
    led_init();

    /* Initialize Buzzer */
    buzzer_init();

    /* Initialize Lock */
    lock_init();

    /* Initialize PN532 Reader */
    pn532_init();

    /* Initialze Non-Volatile Attributes Storage */
    nvs_attributesInit();

    /* Initialize CoAP Protocol */
    ESP_ERROR_CHECK(esp_netif_init());                // Initialize TCP/UDP stack + WIFI (from menu config)
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Initialze event handler for wifi setup
    ESP_ERROR_CHECK(example_connect());

    /* Print Initialization Complete message*/
    lcd_printHome("Initialization");
    lcd_printCustom("Complete", 0, 2);
    delay(3000);

    while (1)
    {
        Serial.println("Waiting for a Card");

        // Print initial message
        lcd_printHome("Waiting for");
        lcd_printCustom("a card", 0, 2);

        // Wait for a card
        bool success = pn532_startExchange();

        if (success)
        {
            // Catch NFC data
            uint8_t *responseByteArray = pn532_getAPDUResponse();
            // char *responseStr = byteArrayToCharArray(responseByteArray);
            Serial.println((char *)responseByteArray); // Print the converted string

            // lcd_setCursor(0, 1);
            lcd_printHome("Card Scanned !\n");

            // Create CoAP request to URI specified in menu config
            CoapTaskParams taskParams;
            taskParams.payload = responseByteArray;
            xTaskCreate(coap_example_client, "coap", 8 * 1024, &taskParams, 5, NULL);

            // Update Occupancy Level

            // String st = nvs_getStringAttribute("location");
            // Serial.print(st);

            Serial.println("CoAP Request completed !");
        }

        if (success)
        {
            Serial.println("Exchange performed");

            led_setRGB(LED_OFF, LED_ON, LED_OFF);
            buzzer_on();
            lock_open();

            // Catch NFC data
            uint8_t *responseByteArray = pn532_getAPDUResponse();
            char *responseStr = byteArrayToCharArray(responseByteArray);
            Serial.println(responseStr); // Print the converted string

            // lcd_setCursor(0, 1);
            lcd_printHome("Card Scanned !\n");

            delay(1000);

            // Turn led off
            led_setRGB(LED_OFF, LED_OFF, LED_OFF);
            buzzer_off();
            lock_close();
            success = false;
            delay(1000);
        }
    }
}

static char *byteArrayToCharArray(uint8_t *byteArray)
{
    char *charArray = (char *)byteArray;
    return charArray;
}