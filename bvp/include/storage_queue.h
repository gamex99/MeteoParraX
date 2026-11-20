#ifndef STORAGE_QUEUE_H
#define STORAGE_QUEUE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class StorageQueue {
public:
    static void begin();
    static void push(const JsonDocument& doc);
    static bool processQueue(const String& stationId, const String& serverUrl, const String& token);
    static int getPendingCount();
};

#endif
