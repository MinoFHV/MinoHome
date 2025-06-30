#pragma once

#include "esp_err.h"

esp_err_t dht20_read_temperature_and_humidity(float *temperature, float *humidity);
esp_err_t dht20_init();
void dht20_measure_and_sendmqtt_task(void *pvParameters);