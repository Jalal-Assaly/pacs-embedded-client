#include "pn532_controller.h"

#include <Arduino.h>
// #include <SPI.h>
#include <Adafruit_PN532.h>

/* Create and Initialize PN532 Reader */
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

/* Global variable UUID */
uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

void pn532_init()
{
    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata)
    {
        Serial.print("Didn't find PN53x board");
        while (1)
            ; // halt
    }

    // Got ok data, print it out!
    Serial.print("Found chip PN5");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);

    nfc.setPassiveActivationRetries(0xFF);

    Serial.println("Waiting for an ISO14443A card");
}

bool pn532_readPassiveTag()
{
    // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
    // 'uid' will be populated with the UID, and uidLength will indicate
    // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    boolean success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

    return success;
}

void pn532_printUUID()
{
    Serial.println("Found a card!");
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        Serial.print(" 0x");
        Serial.print(uid[i], HEX);
    }
    Serial.println("");
}