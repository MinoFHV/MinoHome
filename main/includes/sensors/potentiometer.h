#pragma once

#include "esp_err.h"

esp_err_t adc_init();
esp_err_t adc_potentiometer_read_voltage(float *voltage);
void adc_potentiometer_sendmqtt_task(void *pvParameters);