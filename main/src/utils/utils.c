#include "utils/utils.h"

#include "esp_log.h"

esp_err_t check_esp_err(esp_err_t err, const char *msg, const char *TAG)
{

    if (err != ESP_OK) ESP_LOGE(TAG, "%s failed: %s", msg, esp_err_to_name(err));
    return err;

}