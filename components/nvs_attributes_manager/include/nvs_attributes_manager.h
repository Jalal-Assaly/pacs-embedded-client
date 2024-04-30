#include <WString.h>
#ifndef NVS_ATTRIBUTES_MANAGER_H
#define NVS_ATTRIBUTES_MANAGER_H

#define     ACCESS_POINT_ID          "1"
#define     ACCESS_POINT_LOCATION    "C213"

void nvs_attributesInit();

bool nvs_setStringAttribute(const char* key, const char* value);
const char* nvs_getStringAttribute(const char* key);

bool nvs_setIntAttribute(const char* key, uint64_t value);
uint64_t nvs_getIntAttribute(const char* key);

bool nvs_setBoolAttribute(const char* key, uint8_t value);
bool nvs_getBoolAttribute(const char* key);

bool nvs_eraseAll();

#endif