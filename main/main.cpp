#include <Arduino.h>
#include <cJSON.h>

/* Include Components */
#include <led_controller.h>
#include <buzzer_controller.h>
#include <lock_controller.h>
#include <nvs_attributes_manager.h>
#include <pn532_controller.h>
#include <lcd_controller.h>
#include <button_controller.h>
#include <reed_controller.h>

extern "C"
{
#include <coap_client.h>

#include "esp_wifi.h"
#include "esp_event.h"

#include "protocol_examples_common.h"
}

/* APDU Error Response */
const uint8_t APDU_ERROR_REPONSE[] = {0x6A, 0x82};

/* Admin cards UID */
const uint8_t ADMIN_CARD_UID[] = {0x73, 0x10, 0x92, 0x8A};
const uint8_t ACCESS_CARD_UID[] = {0xD3, 0xD7, 0xD7, 0xE};

/* Create global request and response structures to store payloads */
requestStruct *requestPayload;
responseStruct *responsePayload;

/* Global boolean flag */
bool isPushedForExit = false;

/* Global request and response initialization */
static void requestStruct_init();
static void responseStruct_init();

/* Convert between json and byte arrays and generate json request */
static cJSON *convertByteArrayToJSON(uint8_t *data, size_t length);
static uint8_t *convertJsonToByteArray(cJSON *jsonObj, size_t *requestSize);
static cJSON *createRequestJson(uint8_t *userAttributes, uint8_t userAttributesSize);

/* Get and enforce access decision */
static bool getDecisionValue(cJSON *jsonObj, const char *key);
static void grantAccess(bool isEntering);
static void denyAccess();

/* Interrupt functions */
void isrButton();
void isrReed();

extern "C" void app_main(void)
{
    /* Initialize Arduino as a component */
    initArduino();
    Serial.begin(115200);

    /* Initialize LCD Screen */
    lcd_init();
    lcd_printHome("Device");
    lcd_printCustom("Initialization", 0, 2);

    /* Initialze Non-Volatile Attributes Storage */
    nvs_attributesInit();

    /* Initialize Push button and attack interrupt function */
    button_init(isrButton);

    /* Initialize Reed switch and attach interrupt function */
    reed_init(isrReed);

    /* Initialize LED RGB */
    led_init();

    /* Initialize Buzzer */
    buzzer_init();

    /* Initialize Lock */
    lock_init();

    /* Initialize PN532 Reader */
    pn532_init();

    /* Initialize CoAP Protocol */
    ESP_ERROR_CHECK(esp_netif_init());                // Initialize TCP/UDP stack + WIFI (from menu config)
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Initialze event handler for wifi setup
    ESP_ERROR_CHECK(example_connect());

    // Initialize request and response payloads (allocate memory and verify)
    requestStruct_init();
    responseStruct_init();

    /* Print Initialization Complete message*/
    lcd_printHome("Initialization");
    lcd_printCustom("Complete", 0, 2);
    delay(2000);

    /* Manual check on reed switch after initialization */
    isrReed();

    while (1)
    {
        /* Print initial message */
        lcd_printHome("Waiting for");
        lcd_printCustom("a card", 0, 2);

        /* Attempts to perform APDU exchange with emulated card */
        bool isEmulatedCard = pn532_startAPDUExchange();

        if (isEmulatedCard)
        {
            lcd_printHome("Request Pending...");

            /* Fetch NFC payload and payload size (user attributes) */
            uint8_t *userAttributes = pn532_getAPDUPayload();
            uint8_t userAttributesSize = pn532_getAPDUPayloadSize();

            if (memcmp(userAttributes, APDU_ERROR_REPONSE, sizeof(APDU_ERROR_REPONSE)) == 0)
            {
                denyAccess();
                lcd_printCustom("Unlock Phone", 0, 2);
                continue;
            }

            /* Create JSON request body (user + access point attributes) */
            cJSON *requestJson = createRequestJson(userAttributes, userAttributesSize);

            /* Convert JSON request to byte array */
            size_t requestSize;
            requestPayload->request = convertJsonToByteArray(requestJson, &requestSize);
            requestPayload->requestSize = requestSize;

            /* Send CoAP request */
            coap_send_request(requestPayload, responsePayload);

            /* Convert response to cJSON object */
            cJSON *responseJson = convertByteArrayToJSON(responsePayload->response, responsePayload->responseSize);
            bool accessGranted = getDecisionValue(responseJson, "decision");

            /* Evaluate and grant or deny access */
            accessGranted ? grantAccess(true) : denyAccess();

            /* Delete cJSON objects */
            cJSON_Delete(requestJson);
            cJSON_Delete(responseJson);

            /* Buffer delay (1 sec) between attempts */
            delay(1000);
        }

        else if (!isEmulatedCard && pn532_startUIDExchange())
        {
            /* Attempts to read UID of regular card */
            uint8_t *cardUID = pn532_getUID();

            if (memcmp(cardUID, ADMIN_CARD_UID, sizeof(ADMIN_CARD_UID)) == 0)
            {
                lcd_printHome("Tamper reset !");
                led_setRGB(LED_OFF, LED_OFF, LED_ON);
                for (uint8_t i = 0; i < 2; i++)
                    buzzer_onFor(500);
                led_setRGB(LED_OFF, LED_OFF, LED_OFF);

                /* Set isTampered flag to false in NVS */
                nvs_setBoolAttribute("IT", false);
                attachInterrupt(REED_DIGITAL_INPUT, isrReed, FALLING);

                /* Ensure that magnet is back */
                isrReed();

                /* Buffer delay (1 sec) between attempts */
                delay(1000);
            }
            else if (memcmp(cardUID, ACCESS_CARD_UID, sizeof(ACCESS_CARD_UID)) == 0)
            {
                /* Open door for admin */
                grantAccess(true);

                /* Buffer delay (1 sec) between attempts */
                delay(1000);
            }
            else
            {
                Serial.println("Admin cards not recognized !");
            }
        }
        else if (isPushedForExit)
        {
            lcd_printHome("Works right");
            buzzer_on();
            led_setRGB(0, 0, 255);
            delay(1000);
            buzzer_off();
            led_setRGB(0, 0, 0);
            isPushedForExit = false;
        }
        else
        {
            Serial.println("No card found !");
        }
    }
}

static void requestStruct_init()
{
    // Allocate memory for requestPayload
    requestPayload = (requestStruct *)malloc(sizeof(requestStruct));
    if (requestPayload == NULL)
    {
        Serial.println("Error: Failed to allocate memory for requestPayload");
        return;
    }
    requestPayload->request = NULL; // Initialize pointer to NULL
    requestPayload->requestSize = 0;
}

static void responseStruct_init()
{
    // Allocate memory for responsePayload
    responsePayload = (responseStruct *)malloc(sizeof(responseStruct));
    if (responsePayload == NULL)
    {
        Serial.println("Error: Failed to allocate memory for responsePayload");
        free(requestPayload); // Free previously allocated memory
        return;
    }
    responsePayload->response = NULL; // Initialize pointer to NULL
    responsePayload->responseSize = 0;
}

static cJSON *convertByteArrayToJSON(uint8_t *data, size_t length)
{
    // Convert uint8_t* to a string
    char *jsonString = (char *)malloc(length + 1); // +1 for null terminator
    if (jsonString == NULL)
    {
        return NULL;
    }
    memcpy(jsonString, data, length);
    jsonString[length] = '\0'; // Null terminate the string

    // Parse the string to cJSON object
    cJSON *jsonObj = cJSON_Parse(jsonString);

    if (jsonObj == NULL)
    {
        printf("Error: Failed to parse cJSON object\n");
        return NULL;
    }

    return jsonObj;
}

static uint8_t *convertJsonToByteArray(cJSON *jsonObj, size_t *requestSize)
{
    const int prebufferSize = 1024;
    char *jsonString = cJSON_PrintBuffered(jsonObj, prebufferSize, cJSON_False);

    // Check for printing errors
    if (!jsonString)
    {
        // Handle printing error
        return NULL;
    }

    // Get the length of the JSON string
    size_t jsonLength = strlen(jsonString);

    // Allocate memory for the byte array
    uint8_t *byteArray = (uint8_t *)malloc(jsonLength + 1); // Add one for null terminator
    if (!byteArray)
    {
        cJSON_free(jsonString); // Free cJSON-printed string
        return NULL;
    }

    // Copy the JSON string to the byte array
    memcpy(byteArray, jsonString, jsonLength);
    byteArray[jsonLength] = '\0'; // Add null terminator

    // Assign the size of the request
    *requestSize = jsonLength;

    // Free cJSON-printed string
    cJSON_free(jsonString);

    return byteArray;
}

cJSON *createRequestJson(uint8_t *userAttributes, uint8_t userAttributesSize)
{
    cJSON *receivedPayloadJson = cJSON_CreateObject();
    receivedPayloadJson = convertByteArrayToJSON(userAttributes, userAttributesSize);

    // Extract user attributes and nonce from payload
    cJSON *receivedUAT = cJSON_GetObjectItemCaseSensitive(receivedPayloadJson, "UAT");
    cJSON *receivedNonce = cJSON_GetObjectItemCaseSensitive(receivedPayloadJson, "NC");

    // Make JSON object from access point attributes
    const char *id = nvs_getStringAttribute("ID");
    const char *location = nvs_getStringAttribute("LC");
    bool isTampered = nvs_getBoolAttribute("IT");
    int16_t occupancyLevel = nvs_getIntAttribute("OL");

    // Create cJSON objects for each attribute
    cJSON *accessPointAttributesJson = cJSON_CreateObject();
    cJSON_AddStringToObject(accessPointAttributesJson, "ID", id);
    cJSON_AddStringToObject(accessPointAttributesJson, "LC", location);
    cJSON_AddBoolToObject(accessPointAttributesJson, "IT", isTampered);
    cJSON_AddNumberToObject(accessPointAttributesJson, "OL", occupancyLevel);

    // Create cJSON object for the main structure
    cJSON *requestJson = cJSON_CreateObject();
    cJSON_AddItemToObject(requestJson, "UAT", receivedUAT);
    cJSON_AddItemToObject(requestJson, "APA", accessPointAttributesJson);
    cJSON_AddItemToObject(requestJson, "NC", receivedNonce);

    // Print the JSON string (for testing)
    char *jsonString = cJSON_Print(requestJson);
    printf("%s\n", jsonString);

    // Return the cJSON object
    return requestJson;
}

static bool getDecisionValue(cJSON *jsonObj, const char *key)
{
    if (jsonObj != NULL)
    {
        // Fetch the value associated with the key "decision"
        cJSON *value = cJSON_GetObjectItem(jsonObj, key);

        // Check if the value exists and is a boolean
        if (cJSON_IsBool(value))
        {
            return cJSON_IsTrue(value);
        }
        else
        {
            printf("Value is not a boolean\n");
        }
    }
    else
    {
        printf("JSON object is NULL\n");
    }

    return false;
}

static void grantAccess(bool isEntering)
{
    lcd_printHome("Access Granted !");

    // Update Occupancy Level
    int32_t occupancyLevel = nvs_getIntAttribute("OL");

    // Increment counter if user is entering, otherwise, decrement
    if (isEntering)
    {
        nvs_setIntAttribute("OL", occupancyLevel + 1);
    }
    else
    {
        nvs_setIntAttribute("OL", max(occupancyLevel - 1, 0));
    }

    Serial.print("Occupancy Level: ");
    Serial.println(occupancyLevel);

    // Trigger Actuators
    led_setRGB(LED_OFF, LED_ON, LED_OFF);
    buzzer_on();
    lock_open();

    delay(1250);

    // Turn led off
    led_setRGB(LED_OFF, LED_OFF, LED_OFF);
    buzzer_off();
    lock_close();
}

static void denyAccess()
{
    lcd_printHome("Access Denied !");

    led_setRGB(LED_ON, LED_OFF, LED_OFF);
    for (uint8_t i = 0; i < 3; i++)
        buzzer_onFor(200);
    led_setRGB(LED_OFF, LED_OFF, LED_OFF);
}

void isrButton()
{
    bool isPushed = digitalRead(BUTTON_INPUT);

    if (isPushed)
    {
        Serial.println("Push button pressed !! Open door for exit");
        isPushedForExit = true;
    }
}

void isrReed()
{
    /* Double check to ensure consistent readings */
    bool isTampered = digitalRead(REED_DIGITAL_INPUT);

    if (!isTampered)
    {
        Serial.println("Tamper detected !!");
        /* Set isTampered flag to true in NVS */
        nvs_setBoolAttribute("IT", true);

        Serial.println(nvs_getBoolAttribute("IT"));

        /* Detach interrupt so it is triggered only once */
        detachInterrupt(digitalPinToInterrupt(REED_DIGITAL_INPUT));
    }
}