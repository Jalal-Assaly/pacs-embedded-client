#include <WString.h>
#ifndef NVS_ATTRIBUTES_MANAGER_H
#define NVS_ATTRIBUTES_MANAGER_H

#define     ACCESS_POINT_LOCATION    "C213"

void nvs_attributesInit();

bool nvs_setStringAttribute(const char* key, const char* value);
const char* nvs_getStringAttribute(const char* key);

bool nvs_setIntAttribute(const char* key, int16_t value);
int16_t nvs_getIntAttribute(const char* key);

bool nvs_setBoolAttribute(const char* key, int8_t value);
bool nvs_getBoolAttribute(const char* key);

bool nvs_eraseAll();

#endif