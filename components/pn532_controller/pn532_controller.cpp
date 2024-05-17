#include "pn532_controller.h"

#include <Arduino.h>

#if 0
#include <SPI.h>
#include <Adafruit_PN532.h>
Adafruit_PN532 nfc(PN532_SS, &SPI);

#elif 1
#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"

PN532_SPI pn532spi(SPI, PN532_SS);
PN532 nfc(pn532spi);

/* When the number after #elif set as 1, it will be switch to HSU Mode*/
#elif 0
#include <PN532_HSU.h>
#include <PN532.h>

PN532_HSU pn532hsu(Serial1);
PN532 nfc(pn532hsu);

/* When the number after #if & #elif set as 0, it will be switch to SWHSU Mode*/
#elif 0
#include <NfcAdapter.h>
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>

SoftwareSerial SWSerial(PN532_RX, PN532_TX); // RX, TX
PN532_SWHSU pn532swhsu(SWSerial);
PN532 nfc(pn532swhsu);
NfcAdapter nfc2(pn532swhsu);

/* When the number after #if & #elif set as 0, it will be switch to I2C Mode*/
#else
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
#endif

#define RESPONSE_SIZE 255

/* UUID length and value */
uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID

/* ADPU value */
uint8_t SELECT_APDU[] = {
    0x00,                                     /* CLA */
    0xA4,                                     /* INS */
    0x04,                                     /* P1  */
    0x00,                                     /* P2  */
    0x07,                                     /* Length of AID  */
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, /* AID  */
    0x00                                      /* Le  */
};

/* Response value and length for APDU protocol */
uint8_t response[RESPONSE_SIZE];
uint8_t responseLength = RESPONSE_SIZE;

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
    nfc.SAMConfig();

    Serial.println("Waiting for an ISO14443A card\n");
}

bool pn532_startUIDExchange()
{
    return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
}

bool pn532_startAPDUExchange()
{
    Serial.println("Scanning emulated APDU card");
    bool isInListPassiveTag = false;
    bool isEmulatedAPDUCard = false;

    // set shield to inListPassiveTarget
    isInListPassiveTag = nfc.inListPassiveTarget();

    if (isInListPassiveTag)
    {
        // Reset response length and value
        responseLength = RESPONSE_SIZE;
        std::fill_n(response, responseLength, 0);

        // Attempts to send APDU message and receive response
        isEmulatedAPDUCard = nfc.inDataExchange(SELECT_APDU, sizeof(SELECT_APDU), response, &responseLength);

        if (isEmulatedAPDUCard)
        {
            Serial.println("SELECT AID sent");

            Serial.print("NFC payload length: ");
            Serial.println(responseLength);
            Serial.print("NFC payload: ");
            Serial.println((char *)response); // Show response in bytes

            // Release the currently selected target
            nfc.inRelease();
            Serial.println("Target released\n");
        }
        else
        {
            Serial.println("Emulated card found but communication failed !\n");
        }
    }

    return isInListPassiveTag & isEmulatedAPDUCard;
}

uint8_t *pn532_getUID()
{
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

    return uid;
}

uint8_t *pn532_getAPDUPayload()
{
    return response;
}

uint8_t pn532_getAPDUPayloadSize()
{
    return responseLength;
}