#include <Arduino.h>

#include <led_controller.h>
#include <buzzer_controller.h>
#include <lock_controller.h>
#include <nvs_attributes_manager.h>
#include <pn532_controller.h>
#include <lcd_controller.h>
#include <cJSON.h>

extern "C"
{
#include <coap_client.h>

#include "esp_wifi.h"
#include "esp_event.h"

#include "protocol_examples_common.h"
}

/* Define structure to pass and receive values from and to coap file */
typedef struct
{
    uint8_t *payload;
    uint8_t payloadSize;
    uint8_t *response;
    uint8_t responseSize;
    bool received;
} CoapTaskParams;

cJSON *convertUint8ToCJSON(uint8_t *data, size_t length);

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

        /* Print initial message */
        lcd_printHome("Waiting for");
        lcd_printCustom("a card", 0, 2);

        /* Wait for a card */
        bool success = pn532_startExchange();

        if (success)
        {
            /* Create and execute CoAP request */
            lcd_printHome("Request Pending...\n");

            CoapTaskParams taskParams;
            taskParams.payload = pn532_getAPDUPayload();
            taskParams.payloadSize = pn532_getAPDUPayloadSize();

            Serial.println(taskParams.payloadSize);

            xTaskCreate(coap_send_request, "coap", 8 * 1024, &taskParams, 5, NULL);

            // Wait until coap task terminates
            while (!taskParams.received)
                ; // Halt

            /* Print received response to verify from coap request */
            Serial.print("\n\n\nResponse successfully received in main: ");
            Serial.print((char *)taskParams.response);
            Serial.println("");

            // Convert to cJSON object
            cJSON *json = convertUint8ToCJSON(taskParams.response, taskParams.responseSize);

            // Check if conversion was successful
            if (json == NULL)
            {
                printf("Error: Failed to convert uint8_t* to cJSON object\n");
            }
            else
            {
                // Print the JSON object
                char *jsonString = cJSON_Print(json);
                printf("JSON Object: %s\n", jsonString);

                // Free the cJSON object
                cJSON_Delete(json);
            }

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

cJSON *convertUint8ToCJSON(uint8_t *data, size_t length)
{
    // Convert uint8_t* to a string
    char *jsonString = (char *)malloc(length + 1); // +1 for null terminator
    if (jsonString == NULL)
    {
        // Handle memory allocation failure
        return NULL;
    }
    memcpy(jsonString, data, length);
    jsonString[length] = '\0'; // Null terminate the string

    // Parse the string to cJSON object
    cJSON *jsonObj = cJSON_Parse(jsonString);

    // Free the allocated memory
    free(jsonString);

    return jsonObj;
}