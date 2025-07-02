#pragma once

#include <stddef.h>

#include "esp_err.h"

#define MQTT_TOPIC_TEMPERATURE      "home/esp32/temperature"
#define MQTT_TOPIC_HUMIDITY         "home/esp32/humidity"
#define MQTT_TOPIC_POTI             "home/esp32/poti_voltage"
#define MQTT_TOPIC_LIGHT            "home/esp32/lightsensor"
#define MQTT_TOPIC_CO2              "home/esp32/carbondioxide"
#define MQTT_TOPIC_CH2O             "home/esp32/formaldehyde"
#define MQTT_TOPIC_TVOC             "home/esp32/tvoc"
#define MQTT_TOPIC_AIRQUALITY       "home/esp32/airquality"


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