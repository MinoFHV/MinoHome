#include "sdkconfig.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "my_mqtt.h"


static esp_mqtt_client_handle_t mqtt_client = NULL;
static const char *TAG = "MQTT";


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{


    switch (event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected!");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected!");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT Error!");
            break;
        default:
            break;
        
    }

}

int format_float(char *buf, size_t buf_len, const void *value) {
    float f = *(const float *)value;
    return snprintf(buf, buf_len, "%.2f", f);
}

int format_uint16(char *buf, size_t buf_len, const void *value) {
    uint16_t u = *(const uint16_t *)value;
    return snprintf(buf, buf_len, "%u", u);
}


void mqtt_init()
{

    ESP_LOGI(TAG, "%s - %d", CONFIG_MQTT_BROKER_URI, CONFIG_MQTT_BROKER_PORT);

    const esp_mqtt_client_config_t config =
    {
        .broker.address.uri = CONFIG_MQTT_BROKER_URI,
        .broker.address.port = CONFIG_MQTT_BROKER_PORT,
    };

    mqtt_client = esp_mqtt_client_init(&config);
    
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));

}

void sendMQTTpayload(const char *topic, const void *value, formatter_t formatter)
{

    if (mqtt_client == NULL) return;

    char buf[32];
    formatter(buf, sizeof(buf), value);
    
    esp_mqtt_client_publish(mqtt_client, topic, buf, 0, 1, 0);
    ESP_LOGI(TAG, "Published message, topic=%s, data=%s", topic, buf);

}
