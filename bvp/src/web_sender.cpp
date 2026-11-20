#include "web_sender.h"
#include <WiFi.h>
#include "remote_log.h"

bool WebSender::sendData(const MeteoData& data, const String& stationId, const String& serverUrl, const String& token) {
    if (WiFi.status() != WL_CONNECTED) return false;
    StaticJsonDocument<512> doc;
    doc["temp"] = data.temp_out;
    doc["hum"] = data.hum_out;
    doc["pres"] = data.pressure;
    doc["dir"] = String(data.wind_dir);
    doc["vel"] = data.wind_ave;
    doc["rafaga"] = data.wind_gust;
    doc["lluvia_c"] = data.rain_clicks;
    doc["temp_int"] = data.temp_in;
    doc["hum_int"] = data.hum_in;
    doc["fecha_rpi"] = RemoteLog::getTimestamp();
    doc["chip_id"] = (uint32_t)ESP.getEfuseMac();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["firmware"] = "2.0.0-S3";
    String jsonString;
    serializeJson(doc, jsonString);
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, serverUrl + "?estacion=" + stationId);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-ESP-Token", token);
    int code = http.POST(jsonString);
    String response = http.getString();
    http.end();
    client.stop();
    return (code > 0 && response.indexOf("LOGRADO") >= 0);
}

bool WebSender::sendBulk(const JsonArray& bulkData, const String& stationId, const String& serverUrl, const String& token) {
    if (WiFi.status() != WL_CONNECTED) return false;
    String jsonString;
    serializeJson(bulkData, jsonString);
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, serverUrl + "?estacion=" + stationId);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-ESP-Token", token);
    int code = http.POST(jsonString);
    String response = http.getString();
    http.end();
    client.stop();
    return (code > 0 && response.indexOf("LOGRADO") >= 0);
}
