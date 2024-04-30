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

/* Create global request and response structures to store payloads */
requestStruct *requestPayload;
responseStruct *responsePayload;

/* Global request and response initialization */
static void requestStruct_init();
static void responseStruct_init();

/* Convert between json and byte arrays and generate json request */
static cJSON *convertByteArrayToJSON(uint8_t *data, size_t length);
static uint8_t *convertJsonToByteArray(cJSON *jsonObj, size_t *requestSize);
static cJSON *createRequestJson(uint8_t *userAttributes, uint8_t userAttributesSize);

/* Get and enforce access decision */
static bool getDecisionValue(cJSON *jsonObj, const char *key);
static void grantAccess();
static void denyAccess();

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

    // Initialize request and response payloads (allocate memory and verify)
    requestStruct_init();
    responseStruct_init();

    /* Print Initialization Complete message*/
    lcd_printHome("Initialization");
    lcd_printCustom("Complete", 0, 2);
    delay(2000);

    while (1)
    {
        /* Print initial message */
        lcd_printHome("Waiting for");
        lcd_printCustom("a card", 0, 2);

        /* Wait for a card */
        bool success = pn532_startExchange();

        if (success)
        {
            lcd_printHome("Request Pending...");

            /* Fetch NFC payload and payload size (user attributes) */
            uint8_t *userAttributes = pn532_getAPDUPayload();
            uint8_t userAttributesSize = pn532_getAPDUPayloadSize();

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
            accessGranted ? grantAccess() : denyAccess();

            /* Delete cJSON objects */
            cJSON_Delete(requestJson);
            cJSON_Delete(responseJson);
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
    cJSON *userAttributesJson = cJSON_CreateObject();
    userAttributesJson = convertByteArrayToJSON(userAttributes, userAttributesSize);

    // Make JSON object from access point attributes
    const char *id = nvs_getStringAttribute("id");
    const char *location = nvs_getStringAttribute("location");
    bool isTampered = nvs_getBoolAttribute("isTampered");
    uint64_t occupancyLevel = nvs_getIntAttribute("occupancyLevel");

    // Create cJSON objects for each attribute
    cJSON *accessPointAttributesJson = cJSON_CreateObject();
    cJSON_AddStringToObject(accessPointAttributesJson, "id", id);
    cJSON_AddStringToObject(accessPointAttributesJson, "location", location);
    cJSON_AddBoolToObject(accessPointAttributesJson, "isTampered", isTampered);
    cJSON_AddNumberToObject(accessPointAttributesJson, "occupancyLevel", occupancyLevel);

    // Create cJSON object for the main structure
    cJSON *requestJson = cJSON_CreateObject();
    cJSON_AddItemToObject(requestJson, "userAttributes", userAttributesJson);
    cJSON_AddItemToObject(requestJson, "accessPointAttributes", accessPointAttributesJson);

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

static void grantAccess()
{
    lcd_printHome("Access Granted !");

    // Update Occupancy Level
    uint64_t occupancyLevel = nvs_getIntAttribute("occupancyLevel");
    nvs_setIntAttribute("occupancyLevel", occupancyLevel + 1);

    Serial.print("Occupancy Level: ");
    Serial.println(occupancyLevel);

    // Trigger Actuators
    led_setRGB(LED_OFF, LED_ON, LED_OFF);
    buzzer_on();
    lock_open();

    delay(1000);

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
    {
        buzzer_onFor(200);
    }
    led_setRGB(LED_OFF, LED_OFF, LED_OFF);
}