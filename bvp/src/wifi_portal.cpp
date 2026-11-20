#include "wifi_portal.h"
#include <WiFi.h>
#include "config.h"
#include "remote_log.h"

Preferences WifiPortal::prefs;
WebServer WifiPortal::server(80);
DNSServer WifiPortal::dnsServer;
unsigned long WifiPortal::_portalStartTime = 0;

void WifiPortal::begin() {
    prefs.begin("meteo-s3", false);
}

String WifiPortal::getSSID() { return prefs.getString("wifi_ssid", ""); }
String WifiPortal::getPass() { return prefs.getString("wifi_pass", ""); }
String WifiPortal::getStationID() { return prefs.getString("station_id", "peyrano"); }
String WifiPortal::getServerURL() { return prefs.getString("server_url", DEFAULT_SERVER_URL); }
int WifiPortal::getSendInterval() { return prefs.getInt("send_interval", DEFAULT_SEND_INTERVAL); }

bool WifiPortal::connect() {
    String ssid = getSSID();
    if (ssid == "") return false;
    RemoteLog::logInfo("Connecting to WiFi: " + ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), getPass().c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
        delay(500);
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        RemoteLog::logInfo("WiFi Connected. IP: " + WiFi.localIP().toString());
        return true;
    }
    return false;
}

void WifiPortal::startPortal() {
    RemoteLog::logWarn("Starting Config Portal...");
    WiFi.mode(WIFI_AP);
    String apName = "Meteo-" + getStationID();
    WiFi.softAP(apName.c_str(), PORTAL_PASS);
    dnsServer.start(53, "*", "192.168.4.1");
    server.on("/", []() {
        String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family:sans-serif;padding:20px;background:#f4f4f4} input{width:100%;padding:10px;margin:10px 0}</style></head><body>";
        html += "<h2>Meteo S3 Config</h2>";
        html += "<form action='/save' method='POST'>";
        html += "SSID: <input name='ssid' value='" + getSSID() + "'><br>";
        html += "Pass: <input name='pass' type='password' value='" + getPass() + "'><br>";
        html += "Station ID: <input name='sid' value='" + getStationID() + "'><br>";
        html += "Server URL: <input name='url' value='" + getServerURL() + "'><br>";
        html += "Interval (s): <input name='int' type='number' value='" + String(getSendInterval()) + "'><br>";
        html += "<input type='submit' value='Save & Restart' style='background:#007bff;color:white;border:none;padding:15px;cursor:pointer'>";
        html += "</form></body></html>";
        server.send(200, "text/html", html);
    });
    server.on("/save", []() {
        prefs.putString("wifi_ssid", server.arg("ssid"));
        prefs.putString("wifi_pass", server.arg("pass"));
        prefs.putString("station_id", server.arg("sid"));
        prefs.putString("server_url", server.arg("url"));
        prefs.putInt("send_interval", server.arg("int").toInt());
        server.send(200, "text/html", "Saved! Restarting...");
        delay(2000);
        ESP.restart();
    });
    server.begin();
    _portalStartTime = millis();
}

void WifiPortal::stopPortal() {
    server.stop();
    dnsServer.stop();
    WiFi.mode(WIFI_STA);
}

bool WifiPortal::isPortalActive() {
    return (WiFi.getMode() == WIFI_AP);
}

void WifiPortal::forcePortal() {
    startPortal();
}

void WifiPortal::handleClient() {
    server.handleClient();
}
