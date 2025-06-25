#include "esp_err.h"

esp_err_t dht20_read_temperature_and_humidity(float *temperature, float *humidity);
void dht20_init();