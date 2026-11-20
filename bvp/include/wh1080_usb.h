#ifndef WH1080_USB_H
#define WH1080_USB_H

#include <Arduino.h>
#include "config.h"

struct MeteoData {
    float temp_in;
    float hum_in;
    float temp_out;
    float hum_out;
    float pressure;
    float wind_ave;
    float wind_gust;
    float wind_dir;
    uint16_t rain_clicks;
    bool sensor_error;
    bool connected;
};

class WH1080_USB {
public:
    WH1080_USB();
    bool begin();
    bool update(); // Retorna true si hubo una lectura exitosa
    MeteoData getData();
    bool isConnected();

private:
    bool readBlock(uint16_t addr, uint8_t* buffer);
    void decodeData(uint8_t* buf, MeteoData& data);

    MeteoData _currentData;
    bool _connected = false;

    // USB Host handles (Internal ESP-IDF)
    void* _device_handle = nullptr;
};

#endif
