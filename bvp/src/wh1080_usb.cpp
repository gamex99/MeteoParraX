#include "wh1080_usb.h"
#include "usb/usb_host.h"
#include "esp_log.h"

WH1080_USB::WH1080_USB() {
    _currentData = {0};
}

bool WH1080_USB::begin() {
    usb_host_config_t host_config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    esp_err_t err = usb_host_install(&host_config);
    if (err != ESP_OK) return false;
    return true;
}

bool WH1080_USB::readBlock(uint16_t addr, uint8_t* buffer) {
    // Simulador de lectura (Stub)
    // En una implementación real, aquí se usaría usb_host_transfer_handle_t

    if (addr == 0x0000) {
        // Simular bloque 0: puntero a datos en bytes 30-31
        memset(buffer, 0, 32);
        buffer[30] = 0x10;
        buffer[31] = 0x00;
    } else {
        // Simular bloque de datos con valores razonables
        memset(buffer, 0, 32);
        buffer[1] = 45; // hum_in
        buffer[2] = 220; buffer[3] = 0; // temp_in: 22.0
        buffer[4] = 60; // hum_out
        buffer[5] = 150; buffer[6] = 0; // temp_out: 15.0
        buffer[7] = 101; buffer[8] = 12; // pressure: 101.3 * 10 = 1013
        buffer[9] = 10; // wind_ave_low
        buffer[10] = 20; // wind_gust_low
        buffer[11] = 0x01; // wind_highs
        buffer[12] = 0x04; // wind_dir
        buffer[13] = 100; buffer[14] = 0; // rain_clicks
        buffer[15] = 0x00; // no error
    }
    return true;
}

void WH1080_USB::decodeData(uint8_t* buf, MeteoData& data) {
    data.hum_in = buf[1];
    int16_t rawInTemp = (buf[2] | (buf[3] << 8));
    data.temp_in = (rawInTemp & 0x7FFF) * 0.1f * ((rawInTemp & 0x8000) ? -1.0f : 1.0f);
    data.hum_out = buf[4];
    int16_t rawOutTemp = (buf[5] | (buf[6] << 8));
    data.temp_out = (rawOutTemp & 0x7FFF) * 0.1f * ((rawOutTemp & 0x8000) ? -1.0f : 1.0f);
    uint16_t rawPres = (buf[7] | (buf[8] << 8));
    data.pressure = rawPres * 0.1f;
    uint8_t wind_ave_low = buf[9];
    uint8_t wind_gust_low = buf[10];
    uint8_t wind_highs = buf[11];
    float ave_ms = ((wind_highs & 0x0F) << 8 | wind_ave_low) * 0.1f;
    float gust_ms = (((wind_highs >> 4) & 0x0F) << 8 | wind_gust_low) * 0.1f;
    data.wind_ave = ave_ms * 3.6f;
    data.wind_gust = gust_ms * 3.6f;
    data.wind_dir = (buf[12] & 0x0F) * 22.5f;
    data.rain_clicks = (buf[13] | (buf[14] << 8));
    data.sensor_error = (buf[15] & 0x40) != 0;
}

bool WH1080_USB::update() {
    uint8_t block0[32];
    if (!readBlock(0x0000, block0)) {
        _connected = false;
        return false;
    }
    uint16_t current_pos = block0[30] | (block0[31] << 8);
    uint8_t dataBlock[32];
    if (!readBlock(current_pos, dataBlock)) {
        _connected = false;
        return false;
    }
    _connected = true;
    decodeData(dataBlock, _currentData);
    return true;
}

MeteoData WH1080_USB::getData() {
    return _currentData;
}

bool WH1080_USB::isConnected() {
    return _connected;
}
