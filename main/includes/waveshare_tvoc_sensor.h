#pragma once

#include "esp_err.h"

typedef enum
{
    SENSOR_OK = 0,
    SENSOR_ERROR_HEADER_FOOTER = 1,
    SENSOR_ERROR_CHECKSUM = 2,
    SENSOR_ERROR_TIMEOUT = 3
} tvoc_sensor_uart_status_t;


esp_err_t uart_init();
esp_err_t tvoc_set_active_mode();
tvoc_sensor_uart_status_t read_co2_ch2o_tvoc_airquality(uint8_t *air_quality, uint16_t *co2, uint16_t *ch2o, float *tvoc);
void tvoc_sensor_measure_and_sendmqtt_task(void *pvParameters);