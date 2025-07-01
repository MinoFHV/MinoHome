#include "wireless-protocol-modules/mqtt.h"
#include "utils/utils.h"

#include "sdkconfig.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define MQTT_CONNECTED_BIT BIT0


static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static EventGroupHandle_t mqtt_event_group;


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{

    switch (event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected!");
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT Disconnected!");
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT Error!");
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        default:
            break;
    }
    
}

int format_float(char *buf, size_t buf_len, const void *value)
{
    float f = *(const float *)value;
    return snprintf(buf, buf_len, "%.2f", f);
}

int format_uint8(char *buf, size_t buf_len, const void *value)
{
    uint8_t u = *(const uint8_t *)value;
    return snprintf(buf, buf_len, "%u", u);
}

int format_uint16(char *buf, size_t buf_len, const void *value)
{
    uint16_t u = *(const uint16_t *)value;
    return snprintf(buf, buf_len, "%u", u);
}

esp_err_t mqtt_init()
{

    mqtt_event_group = xEventGroupCreate();
    if (mqtt_event_group == NULL)
    {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    const esp_mqtt_client_config_t config =
    {
        .broker.address.uri = CONFIG_MQTT_BROKER_URI,
        .broker.address.port = CONFIG_MQTT_BROKER_PORT,
    };

    mqtt_client = esp_mqtt_client_init(&config);
    if (mqtt_client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }
    
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_err_t ret = esp_mqtt_client_start(mqtt_client);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start MQTT client");
        return ret;
    }

    // Wait for connection event or timeout (10 seconds)
    EventBits_t bits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(10000));

    if (bits & MQTT_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "MQTT connected successfully");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "MQTT connection timed out or failed");
        return ESP_ERR_TIMEOUT;
    }

}

mqtt_status_t sendMQTTpayload(const char *topic, const void *value, formatter_t formatter)
{

    if (mqtt_client == NULL) return MQTT_NO_CLIENT;

    char buf[32];
    formatter(buf, sizeof(buf), value);
    
    int ret = esp_mqtt_client_publish(mqtt_client, topic, buf, 0, 1, 0);
    if (ret >= 0)
    {
        ESP_LOGI(TAG, "Published message, topic=%s, data=%s", topic, buf);
        return MQTT_OK;
    }
    else if (ret == -1)
    {
        ESP_LOGE(TAG, "MQTT Error: Failure publishing!");
        return MQTT_FAILURE;
    } 
    else if (ret == -2)
    {
        ESP_LOGE(TAG, "MQTT Error: Full outbox!");
        return MQTT_FULL_OUTBOX;
    }

    return MQTT_UNKNOWN_ERROR;

}
