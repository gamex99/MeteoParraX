#include "ota_telegram.h"
#include "remote_log.h"
#include "wifi_portal.h"

unsigned long _lastPolling = 0;

void OtaTelegram::begin() {}

void OtaTelegram::sendMsg(const String& msg) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage?chat_id=" + String(AUTHORIZED_CHAT_ID) + "&text=" + msg;
    http.begin(client, url);
    http.GET();
    http.end();
    client.stop();
}

void OtaTelegram::update() {
    if (millis() - _lastPolling < 30000) return;
    _lastPolling = millis();
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/getUpdates?offset=-1";
    http.begin(client, url);
    int code = http.GET();
    if (code == 200) {
        String body = http.getString();
        if (body.indexOf("/ota " + WifiPortal::getStationID()) >= 0) {
            sendMsg("🚀 Iniciando actualización OTA para " + WifiPortal::getStationID());
            String binUrl = String(GITHUB_REPO_BASE) + "/" + WifiPortal::getStationID() + "/firmware.bin";
            HTTPClient updateClient;
            updateClient.begin(client, binUrl);
            int updateCode = updateClient.GET();
            if (updateCode == 200) {
                size_t size = updateClient.getSize();
                Update.begin(size);
                WiFiClientSecure* stream = (WiFiClientSecure*)updateClient.getStreamPtr();
                size_t written = Update.writeStream(*stream);
                if (Update.end() && written == size) {
                    sendMsg("✅ Actualización exitosa. Reiniciando...");
                    delay(1000);
                    ESP.restart();
                } else {
                    sendMsg("❌ Error al escribir en Flash.");
                }
            } else {
                sendMsg("❌ No se encontró el binario en GitHub.");
            }
        }
    }
    http.end();
    client.stop();
}

void OtaTelegram::sendLogFile() {
    // Implementación simplificada para envío de documento
}
