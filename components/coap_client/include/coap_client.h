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

coap_response_t message_handler(coap_session_t *session, const coap_pdu_t *sent, const coap_pdu_t *received, const coap_mid_t mid);
void coap_log_handler(coap_log_t level, const char *message);
coap_address_t *coap_get_address(coap_uri_t *uri);
int coap_build_optlist(coap_uri_t *uri);
void coap_send_request(void* params);

#endif /* COAP_CLIENT_H */