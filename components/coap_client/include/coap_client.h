#ifndef COAP_CLIENT_H
#define COAP_CLIENT_H

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"

#include "coap3/coap.h"

/* Define structure to pass and receive requests and responses from and to main */
typedef struct
{
    uint8_t *request;
    size_t requestSize;
} requestStruct;

typedef struct
{
    uint8_t *response;
    size_t responseSize;
} responseStruct;

void coap_send_request(requestStruct* requestPayload, responseStruct *responsePayload);

#endif /* COAP_CLIENT_H */