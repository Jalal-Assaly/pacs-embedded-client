#include <WString.h>
#ifndef NVS_ATTRIBUTES_MANAGER_H
#define NVS_ATTRIBUTES_MANAGER_H

void nvs_attributesInit();

bool nvs_setStringAttribute(String key, String value);
String nvs_getStringAttribute(String key);

bool nvs_setIntAttribute(String key, uint64_t value);
uint64_t nvs_getIntAttribute(String key);

#endif