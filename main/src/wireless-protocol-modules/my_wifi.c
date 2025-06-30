#include "wireless-protocol-modules/my_wifi.h"
#include "utils/utils.h"

#include "sdkconfig.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


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

esp_err_t wifi_init_and_connect()
{

    // Remove WiFi Connection upon reboot
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);

    if (ret == ESP_OK)
    {

        ESP_LOGI(TAG, "Disconnecting from WiFi...");
        
        ret = check_esp_err(esp_wifi_disconnect(), "esp_wifi_disconnect", TAG);
        if (ret != ESP_OK) return ret;

    }
    else
    {
        ESP_LOGI(TAG, "Not connected to any WiFi network, cannot disconnect.");
    }

    // WiFi Init
    ret = check_esp_err(esp_netif_init(), "esp_netif_init", TAG);
    if (ret != ESP_OK) return ret;

    ret = check_esp_err(esp_event_loop_create_default(), "esp_event_loop_create_default", TAG);
    if (ret != ESP_OK) return ret;
    
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ret = check_esp_err(esp_wifi_init(&wifi_init_config), "esp_wifi_init", TAG);
    if (ret != ESP_OK) return ret;

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

    ret = check_esp_err(esp_wifi_set_mode(WIFI_MODE_STA), "esp_wifi_set_mode", TAG);
    if (ret != ESP_OK) return ret;

    ret = check_esp_err(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config), "esp_wifi_set_config", TAG);
    if (ret != ESP_OK) return ret;

    // WiFi Start
    ret = check_esp_err(esp_wifi_start(), "esp_wifi_start", TAG);
    if (ret != ESP_OK) return ret;

    // WiFi Connect
    ret = check_esp_err(esp_wifi_connect(), "esp_wifi_connect", TAG);
    if (ret != ESP_OK) return ret;

    // Wait until WiFi connection is established
    vTaskDelay(pdMS_TO_TICKS(3000));

    return ESP_OK;

}