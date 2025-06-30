#pragma once

#include "esp_err.h"

esp_err_t check_esp_err(esp_err_t err, const char *msg, const char *TAG);