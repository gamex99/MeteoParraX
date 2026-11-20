#ifndef WEB_SENDER_H
#define WEB_SENDER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "wh1080_usb.h"

class WebSender {
public:
    static bool sendData(const MeteoData& data, const String& stationId, const String& serverUrl, const String& token);
    static bool sendBulk(const JsonArray& bulkData, const String& stationId, const String& serverUrl, const String& token);
};

#endif
