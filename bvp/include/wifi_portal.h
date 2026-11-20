#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>

class WifiPortal {
public:
    static void begin();
    static bool connect();
    static void startPortal();
    static void stopPortal();
    static bool isPortalActive();
    static void forcePortal();
    static void handleClient();

    static String getSSID();
    static String getPass();
    static String getStationID();
    static String getServerURL();
    static int getSendInterval();

private:
    static Preferences prefs;
    static WebServer server;
    static DNSServer dnsServer;
    static unsigned long _portalStartTime;
};

#endif
