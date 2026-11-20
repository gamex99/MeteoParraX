#include "storage_queue.h"
#include "web_sender.h"
#include "remote_log.h"

void StorageQueue::begin() {
    if (!LittleFS.begin(true)) {
        RemoteLog::logError("LittleFS fail in StorageQueue");
    }
}

void StorageQueue::push(const JsonDocument& doc) {
    String filename = "/q_" + String(millis()) + ".json";
    File f = LittleFS.open(filename, "w");
    if (f) {
        serializeJson(doc, f);
        f.close();
    }
}

int StorageQueue::getPendingCount() {
    int count = 0;
    File root = LittleFS.open("/", "r");
    while (File file = root.openNextFile()) {
        if (String(file.name()).startsWith("/q_")) count++;
        file.close();
    }
    return count;
}

bool StorageQueue::processQueue(const String& stationId, const String& serverUrl, const String& token) {
    File root = LittleFS.open("/", "r");
    if (!root) return false;

    while (true) {
        File file = root.openNextFile();
        if (!file) break;

        String filename = file.name();
        if (String(file.name()).startsWith("/q_")) {
            file.close();
            continue;
        }

        DynamicJsonDocument bulkDoc(8192);
        JsonArray array = bulkDoc.to<JsonArray>();
        String filesToDelete[10];
        int count = 0;

        // Process in batches of 10
        do {
            String content = file.readString();
            file.close();

            DynamicJsonDocument tempDoc(1024);
            DeserializationError err = deserializeJson(tempDoc, content);
            if (!err) {
                JsonObject obj = array.createNestedObject();
                for (JsonPair p : tempDoc.as<JsonObject>()) {
                    obj[p.key()] = p.value();
                }
                filesToDelete[count++] = filename;
            }

            if (count >= 10) break;

            file = root.openNextFile();
            if (!file) break;
            filename = file.name();
            if (String(file.name()).startsWith("/q_")) {
                file.close();
                file = root.openNextFile();
                if (!file) break;
                filename = file.name();
            }
        } while (file);

        if (file) file.close();

        if (count > 0) {
            if (WebSender::sendBulk(array, stationId, serverUrl, token)) {
                for (int i = 0; i < count; i++) {
                    String path = filesToDelete[i];
                    if (!path.startsWith("/")) path = "/" + path;
                    LittleFS.remove(path);
                }
            } else {
                // Server unreachable or rejected bulk, stop processing queue
                break;
            }
        }
    }
    root.close();
    return true;
}
