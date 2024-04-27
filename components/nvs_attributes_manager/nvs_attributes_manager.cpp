#include "nvs_attributes_manager.h"
#include "non_volatile_storage.h"

void nvs_attributesInit() {
    NVS.begin("attributes");

    // NVS.setString("id", "1");
    // NVS.setString("location", "Workspace");
    // NVS.setString("isTampered", "false");
}

bool nvs_setStringAttribute(String key, String value) {
    bool success;
    
    if (NVS.getString(key) != NULL) {
        success = NVS.setString(key, value);
    }
    else {
        success = false;
    }
    
    return success;
}
String nvs_getStringAttribute(String key) {
    return NVS.getString(key);
}

bool nvs_setIntAttribute(String key, uint64_t value) {
    bool success;
    
    if (NVS.getInt(key) != NULL) {
        success = NVS.setInt(key, value);
    }
    else {
        success = false;
    }
    
    return success;
}

uint64_t nvs_getIntAttribute(String key) {
    return NVS.getInt(key);
}