#include <esp_wifi.h>

// Arduino's prebuilt core hardcodes 32 dynamic RX/TX buffers. Bound the
// supported IDF initialization settings here so network bursts leave room for
// HUB75 DMA and Classic Bluetooth on the ESP32 without PSRAM. The linker wrap
// applies to Arduino's call without modifying PlatformIO's cached framework.
extern "C" esp_err_t __real_esp_wifi_init(const wifi_init_config_t* config);
extern "C" esp_err_t __wrap_esp_wifi_init(const wifi_init_config_t* config) {
  if (!config) return __real_esp_wifi_init(config);
  wifi_init_config_t bounded = *config;
  bounded.static_rx_buf_num = 2;
  bounded.dynamic_rx_buf_num = 4;
  bounded.tx_buf_type = 1;
  bounded.static_tx_buf_num = 0;
  bounded.dynamic_tx_buf_num = 4;
  bounded.cache_tx_buf_num = 4;
  bounded.rx_ba_win = 2;
  return __real_esp_wifi_init(&bounded);
}
