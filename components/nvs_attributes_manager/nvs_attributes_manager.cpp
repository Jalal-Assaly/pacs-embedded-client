#include "nvs_attributes_manager.h"
#include "non_volatile_storage.h"

void nvs_attributesInit()
{
    NVS.begin("attributes");

    // Set static attributes of access point (never changed)
    NVS.setString("id", ACCESS_POINT_ID);
    NVS.setString("location", ACCESS_POINT_LOCATION);
    NVS.setInt("isTampered", (uint8_t)false);
    
    // Set dynamic attributes of access point (changed with access attempts)
    if (!NVS.getInt("occupancyLevel")) NVS.setInt("occupancyLevel", 0);
}

bool nvs_setStringAttribute(const char* key, char* value)
{
    return NVS.setString(key, value);
}

const char* nvs_getStringAttribute(const char* key)
{
    return strdup(NVS.getString(key).c_str());
}

bool nvs_setIntAttribute(const char* key, uint64_t value)
{
    return NVS.setInt(key, value);
}

uint64_t nvs_getIntAttribute(const char* key)
{
    return NVS.getInt(key);
}

bool nvs_setBoolAttribute(const char* key, uint8_t value) 
{
    return NVS.setInt(key, value);
}

bool nvs_getBoolAttribute(const char* key)
{
    return (bool)NVS.getInt(key);
}


bool nvs_eraseAll() {
    return NVS.eraseAll();
}