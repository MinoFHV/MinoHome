#include "sdkconfig.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "my_wifi.h"

static const char *TAG = "WIFI";

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

    switch (event_id)
    {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WiFi connecting...");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "WiFi connected!");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "WiFi lost connection...");
            wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
            ESP_LOGW(TAG, "WiFi lost connection. Reason: %d", event->reason);
            break;
        default:
            break;
    }

}

// ToDo, return esp_err_t !
void wifi_init_and_connect()
{

    // Reset existing WiFi state (in case of reboot)
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();

    // WiFi Init
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    // Wifi Configuration
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_config_t wifi_config =
    {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASS,
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_WPA_PSK
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // WiFi Start
    ESP_ERROR_CHECK(esp_wifi_start());

    // WiFi Connect
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Wait until WiFi connection is established
    vTaskDelay(pdMS_TO_TICKS(3000));

}

void wifi_disconnect()
{

    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Disconnecting from WiFi...");
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }
    else
    {
        ESP_LOGI(TAG, "Not connected to any WiFi network, cannot disconnect.");
    }

}