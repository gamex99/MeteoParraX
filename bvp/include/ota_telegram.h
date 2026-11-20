#ifndef OTA_TELEGRAM_H
#define OTA_TELEGRAM_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include "config.h"

class OtaTelegram {
public:
    static void begin();
    static void update();
    static void sendMsg(const String& msg);
    static void sendLogFile();
};

#endif
