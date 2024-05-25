#include "nvs_attributes_manager.h"
#include "non_volatile_storage.h"

void nvs_attributesInit()
{
    NVS.begin("attributes");

    // Set Location Attribute
    NVS.setString("LC", ACCESS_POINT_LOCATION);
    
    // Set dynamic attributes of access point (changed with each access attempts)
    int8_t itValue = NVS.getInt("IT", -1);
    if (itValue == -1) NVS.setInt("IT", (int8_t)false);
    
    int16_t olValue = NVS.getInt("OL", -1);
    if (olValue == -1) NVS.setInt("OL", 0);
}

bool nvs_setStringAttribute(const char* key, char* value)
{
    return NVS.setString(key, value);
}

const char* nvs_getStringAttribute(const char* key)
{
    return strdup(NVS.getString(key).c_str());
}

bool nvs_setIntAttribute(const char* key, int16_t value)
{
    return NVS.setInt(key, value);
}

int16_t nvs_getIntAttribute(const char* key)
{
    return NVS.getInt(key);
}

bool nvs_setBoolAttribute(const char* key, int8_t value) 
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