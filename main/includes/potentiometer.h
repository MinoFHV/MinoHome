#pragma once

#include "esp_log.h"

esp_err_t adc_init();
esp_err_t adc_potentiometer_read_voltage(float *voltage);