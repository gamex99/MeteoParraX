#ifndef REMOTE_LOG_H
#define REMOTE_LOG_H

#include <Arduino.h>
#include <LittleFS.h>

class RemoteLog {
public:
    static void begin();
    static void logInfo(const String& msg);
    static void logWarn(const String& msg);
    static void logError(const String& msg);
    static String getStatus();
    static String getLastLines(int lines = 35);
    static bool sendLogFile();
    static void clearLogs();
    static String getTimestamp();

private:
    static void writeToFile(const String& level, const String& msg);
    static void rotateLogs();
};

#endif