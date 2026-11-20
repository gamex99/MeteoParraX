#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// CONFIGURACIÓN GLOBAL (Fija para todas las estaciones)
// =============================================================================

// Telegram API
const char* const TELEGRAM_BOT_TOKEN = "9097280230:AAE5_C1w1qAzpXW2PwNN74C0G43OXrUXUqo";
const char* const AUTHORIZED_CHAT_ID  = "5249550006";

// GitHub OTA (Rama main)
const char* const GITHUB_REPO_BASE    = "https://raw.githubusercontent.com/gamex99/MeteoParraX/main";

// Backend
const char* const DEFAULT_SERVER_URL  = "https://gamex99.bsite.net/receptor.aspx";

// Portal Captivo
const char* const PORTAL_SSID         = "MeteoESP32-Setup";
const char* const PORTAL_PASS         = "12345678";
const unsigned long PORTAL_TIMEOUT = 180000; // 3 minutos (Seguridad corte luz)

// Configuración de Hardware USB WH1080 / Daza DZWT1081
const uint16_t WH1080_VID = 0x1941;
const uint16_t WH1080_PID = 0x8021;
const uint8_t  WH1080_REG_ADDR = 0x0000; // Bloque base para leer puntero current_pos

// Parámetros de Sistema
const unsigned long DEFAULT_SEND_INTERVAL = 60; // Segundos
const size_t MAX_LOG_SIZE = 50 * 1024;         // 50 KB para rotación de logs

// Hardware S3
const int PIN_BOOT_BUTTON = 0; // Botón BOOT para forzar portal

#endif
