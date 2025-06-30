#pragma once

#include <stddef.h>

#include "esp_err.h"


typedef enum
{
    MQTT_UNKNOWN_ERROR = -1,
    MQTT_OK = 0,
    MQTT_NO_CLIENT = 1,
    MQTT_FAILURE = 2,
    MQTT_FULL_OUTBOX = 3
} mqtt_status_t;

typedef int (*formatter_t)(char *buf, size_t buf_len, const void *value);


int format_float(char *buf, size_t buf_len, const void *value);
int format_uint8(char *buf, size_t buf_len, const void *value);
int format_uint16(char *buf, size_t buf_len, const void *value);

esp_err_t mqtt_init();
mqtt_status_t sendMQTTpayload(const char *topic, const void *value, formatter_t formatter);