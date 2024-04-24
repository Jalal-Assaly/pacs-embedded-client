#include <Arduino.h>

#include <led_controller.h>
#include <buzzer_controller.h>
#include <lock_controller.h>
#include <non_volatile_storage.h>
#include <pn532_controller.h>
#include <lcd_controller.h>

extern "C"
{
#include <coap_client.h>

#include "esp_wifi.h"
#include "esp_event.h"

#include "protocol_examples_common.h"
}

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

    /* Initialize CoAP Protocol */
    ESP_ERROR_CHECK(esp_netif_init()); // Initialize TCP/UDP stack + WIFI (from menu config)
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Initialze event handler for wifi setup
    ESP_ERROR_CHECK(example_connect());

    /* Initialze Non-Volatile Storage */
    NVS.begin("attributes");
    NVS.setString("id", "1");
    NVS.setString("location", "Workspace");
    NVS.setString("isTampered", "false");
    NVS.setInt("occupancyLevel", NULL);

    lcd_printHome("Initialization");
    lcd_printCustom("Complete", 0, 2);
    delay(3000);

    while (1)
    {
        // Print initial message
        lcd_printHome("Waiting for");
        lcd_printCustom("a card", 0, 2);

        // Wait for a card
        boolean success = pn532_readPassiveTag();

        if (success)
        {
            led_setRGB(LED_OFF, LED_ON, LED_OFF);
            buzzer_on();
            lock_open();

            // Print UUID
            pn532_printUUID();
            lcd_setCursor(0,1);
            lcd_printHome("Card Scanned !");
            // Wait 1 second before continuing
            
            // write to flash
            const String st_set = "simple plain string";
            bool res = NVS.setString("st", st_set);
            // read from flash
            String st = NVS.getString("st");

            Serial.print(st);

            delay(1000);

            // Turn led off
            led_setRGB(LED_OFF, LED_OFF, LED_OFF);
            buzzer_off();
            lock_close();
            delay(1000);
        }
        else
        {
            Serial.print("No tag found");
        }

        // Create CoAP request to URI specified in menu config
        xTaskCreate(coap_example_client, "coap", 8 * 1024, NULL, 5, NULL);
    }
}
