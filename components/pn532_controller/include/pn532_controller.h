#ifndef PN532_CONTROLLER_H
#define PN532_CONTROLLER_H

#define     PN532_SCK       (15)
#define     PN532_MISO      (2)
#define     PN532_MOSI      (4)
#define     PN532_SS        (16)

void pn532_init();
bool pn532_readPassiveTag();
void pn532_printUUID();

#endif