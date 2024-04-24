#ifndef NON_VOLATILE_STORAGE_H
#define NON_VOLATILE_STORAGE_H

#include <Arduino.h>
#include <vector>

extern "C" {
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
}

#ifndef ARDUINONVS_SILENT
#define ARDUINONVS_SILENT 0
#endif

#if ARDUINONVS_SILENT
  #define DEBUG_PRINT(...) { }
  #define DEBUG_PRINTLN(...) { }
  #define DEBUG_PRINTF(fmt, args...) { }
#else
  #define DEBUG_PRINTER Serial
  #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
  #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
  #define DEBUG_PRINTF(fmt, args...) { DEBUG_PRINTER.printf(fmt,## args); }
#endif

class ArduinoNvs {
public:
  ArduinoNvs();

  bool    begin(String namespaceNvs = "storage");
  void    close();

  bool    eraseAll(bool forceCommit = true);
  bool    erase(String key, bool forceCommit = true);

  bool    setInt(String key, uint8_t value, bool forceCommit = true);
  bool    setInt(String key, int16_t value, bool forceCommit = true);
  bool    setInt(String key, uint16_t value, bool forceCommit = true);
  bool    setInt(String key, int32_t value, bool forceCommit = true);
  bool    setInt(String key, uint32_t value, bool forceCommit = true);
  bool    setInt(String key, int64_t value, bool forceCommit = true);
  bool    setInt(String key, uint64_t value, bool forceCommit = true);
  bool    setFloat(String key, float value, bool forceCommit = true);
  bool    setString(String key, String value, bool forceCommit = true);
  bool    setBlob(String key, uint8_t* blob, size_t length, bool forceCommit = true);
  bool    setBlob(String key, std::vector<uint8_t>& blob, bool forceCommit = true);

  int64_t getInt(String key, int64_t default_value = 0);  // In case of error, default_value will be returned
  float   getFloat(String key, float default_value = 0);
  
  bool    getString(String key, String& res);
  String  getString(String key);

  size_t  getBlobSize(String key);  /// Returns the size of the stored blob
  bool    getBlob(String key,  uint8_t* blob, size_t length);  /// User should proivde enought memory to store the loaded blob. If length < than required size to store blob, function fails.
  bool    getBlob(String key, std::vector<uint8_t>& blob);  
  std::vector<uint8_t> getBlob(String key); /// Less eficient but more simple in usage implemetation of `getBlob()`

  bool        commit();
protected:
  nvs_handle  _nvs_handle;    
};

extern ArduinoNvs NVS;

#endif