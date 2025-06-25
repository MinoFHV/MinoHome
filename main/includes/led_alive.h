#pragma once

#include "esp_err.h"

void led_program_alive_task_start(void *pvParameters);
// ToDo: LED Task Error (red blinking)

esp_err_t led_init();