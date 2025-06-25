#pragma once

#include "esp_err.h"

void led_program_alive_task(void *pvParameters);
esp_err_t led_init();