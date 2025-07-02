#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

esp_err_t i2c_master_bus_init();
i2c_master_bus_handle_t i2c_get_master_bus_handle();
SemaphoreHandle_t i2c_get_semaphore();