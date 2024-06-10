#include <cstdint>

#ifndef PN532_CONTROLLER_H
#define PN532_CONTROLLER_H

#define     PN532_SCK       (18)
#define     PN532_MISO      (19)
#define     PN532_MOSI      (23)
#define     PN532_SS        (5)

/* Enum definition */ 
enum APDUResult {
    EMULATED_CARD_FOUND,
    EMULATED_CARD_NOT_FOUND,
    CONNECTION_INTERRUPTED
};

void pn532_init();

bool pn532_startUIDExchange();
APDUResult pn532_startAPDUExchange();
void pn532_setPassiveActivationRetries(int maxRetries);

uint8_t* pn532_getUID();
uint8_t* pn532_getAPDUPayload();
uint8_t pn532_getAPDUPayloadSize();

#endif