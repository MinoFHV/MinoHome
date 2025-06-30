#pragma once

#include "esp_err.h"

esp_err_t bh1750_init();
esp_err_t bh1750_read_lux(float *lux);
void bh1750_measure_and_sendmqtt_task(void *pvParameters);