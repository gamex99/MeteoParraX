#include "remote_log.h"
#include <time.h>
#include <WiFi.h>
#include "config.h"

const char* LOG_FILE = "/system.log";
const char* OLD_LOG_FILE = "/system.old.log";

void RemoteLog::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("CRITICAL: LittleFS failed!");
        return;
    }
    logInfo("--- System Boot ---");
    logInfo("Firmware Version: 2.0.0-S3");
    logInfo("Reset Reason: " + String(esp_reset_reason()));
}

String RemoteLog::getTimestamp() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buf);
}

void RemoteLog::writeToFile(const String& level, const String& msg) {
    String entry = "[" + getTimestamp() + "] [" + level + "] " + msg + "\n";
    if (LittleFS.exists(LOG_FILE)) {
        File f_check = LittleFS.open(LOG_FILE, "r");
        if (f_check && f_check.size() > MAX_LOG_SIZE) {
            f_check.close();
            rotateLogs();
        } else {
            if (f_check) f_check.close();
        }
    }
    File f = LittleFS.open(LOG_FILE, "a");
    if (f) {
        f.print(entry);
        f.close();
    }
}

void RemoteLog::rotateLogs() {
    LittleFS.rename(LOG_FILE, OLD_LOG_FILE);
}

void RemoteLog::logInfo(const String& msg) {
    Serial.println("[INFO] " + msg);
    writeToFile("INFO", msg);
}

void RemoteLog::logWarn(const String& msg) {
    Serial.println("[WARN] " + msg);
    writeToFile("WARN", msg);
}

void RemoteLog::logError(const String& msg) {
    Serial.println("[ERROR] " + msg);
    writeToFile("ERROR", msg);
}

String RemoteLog::getStatus() {
    char buf[256];
    snprintf(buf, sizeof(buf),
        "📊 *Status Station*\n"
        "Uptime: %lu s\n"
        "Heap: %u bytes\n"
        "RSSI: %d dBm\n",
        millis()/1000, ESP.getFreeHeap(), WiFi.RSSI());
    return String(buf);
}

String RemoteLog::getLastLines(int lines) {
    File f = LittleFS.open(LOG_FILE, "r");
    if (!f) return "No log file found.";
    String content = f.readString();
    f.close();
    int pos = 0;
    int count = 0;
    while (pos < content.length() && count < lines) {
        if (content[pos] == '\n') count++;
        pos++;
    }
    if (pos == 0) return content;
    return content.substring(content.length() - pos);
}

bool RemoteLog::sendLogFile() {
    return LittleFS.exists(LOG_FILE);
}

void RemoteLog::clearLogs() {
    LittleFS.remove(LOG_FILE);
    LittleFS.remove(OLD_LOG_FILE);
}
