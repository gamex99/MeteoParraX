#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include "config.h"
#include "wh1080_usb.h"
#include "wifi_portal.h"
#include "remote_log.h"
#include "web_sender.h"
#include "storage_queue.h"
#include "ota_telegram.h"

WH1080_USB usbStation;
unsigned long _lastSend = 0;
unsigned long _portalStart = 0;
unsigned long _lastButtonCheck = 0;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);
    RemoteLog::begin();
    StorageQueue::begin();
    WifiPortal::begin();
    if (!WifiPortal::connect()) {
        WifiPortal::startPortal();
        _portalStart = millis();
    } else {
        // Sincronizar hora via NTP (GMT-3 para Argentina/Uruguay/Chile)
        configTzTime("GMT-3", "pool.ntp.org", "time.nist.gov");
        RemoteLog::logInfo("Sincronizando hora NTP...");
    }
    if (!usbStation.begin()) {
        RemoteLog::logError("Error iniciando USB Host");
    }
    ArduinoOTA.setHostname(("Meteo-" + WifiPortal::getStationID()).c_str());
    ArduinoOTA.begin();
    OtaTelegram::begin();
    RemoteLog::logInfo("Sistema Iniciado Correctamente");
}

void loop() {
    ArduinoOTA.handle();
    if (WifiPortal::isPortalActive()) {
        WifiPortal::handleClient();
        if (millis() - _portalStart > PORTAL_TIMEOUT) {
            RemoteLog::logWarn("Portal Timeout. Reintentando WiFi...");
            WifiPortal::stopPortal();
            WifiPortal::connect();
        }
        return;
    }
    if (usbStation.update()) {
        MeteoData data = usbStation.getData();
        if (millis() - _lastSend > (WifiPortal::getSendInterval() * 1000)) {
            _lastSend = millis();
            if (WebSender::sendData(data, WifiPortal::getStationID(), WifiPortal::getServerURL(), "c56e821df190e16195d5a56a60590152072c")) {
                RemoteLog::logInfo("Datos enviados OK");
                StorageQueue::processQueue(WifiPortal::getStationID(), WifiPortal::getServerURL(), "c56e821df190e16195d5a56a60590152072c");
            } else {
                RemoteLog::logWarn("Envío fallido. Guardando en Cola Offline...");
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
                StorageQueue::push(doc);
            }
        }
    }
    OtaTelegram::update();

    // Verificar botón BOOT cada 100ms para evitar saturar el procesador
    if (millis() - _lastButtonCheck > 100) {
        _lastButtonCheck = millis();
        if (digitalRead(PIN_BOOT_BUTTON) == LOW) {
            unsigned long pressStart = millis();
            while(digitalRead(PIN_BOOT_BUTTON) == LOW) {
                if(millis() - pressStart > 5000) {
                    RemoteLog::logInfo("Botón BOOT presionado. Forzando Portal...");
                    WifiPortal::forcePortal();
                    break;
                }
                delay(10); // Pequeña pausa para estabilidad
            }
        }
    }
}