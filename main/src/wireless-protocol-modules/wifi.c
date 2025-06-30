#include "wireless-protocol-modules/wifi.h"
#include "utils/utils.h"

#include "sdkconfig.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1
#define WIFI_MAX_RETRY 5


static const char *TAG = "WIFI";
static EventGroupHandle_t wifi_event_group;
static uint8_t retry_num = 0;


static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

    if (event_base == WIFI_EVENT)
    {

        switch (event_id)
        {

            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi connecting...");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected!");
                retry_num = 0;
                break;
            case WIFI_EVENT_STA_DISCONNECTED:

                if (retry_num < WIFI_MAX_RETRY)
                {
                    ESP_LOGW(TAG, "WiFi disconnected, trying to reconnect...");
                    esp_wifi_connect();
                    retry_num++;
                    ESP_LOGI(TAG, "Retrying to connect to WiFi (%d/%d)", retry_num, WIFI_MAX_RETRY);
                }
                else
                {
                    xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                }

                break;

            default:
                break;

        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    
    {
        ESP_LOGI(TAG, "Got IP address");
        retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

    }

}

esp_err_t wifi_init_and_connect()
{

    wifi_event_group = xEventGroupCreate();

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
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdTRUE,    // clear bits on exit
                                           pdFALSE,   // wait for any bit
                                           pdMS_TO_TICKS(10000));  // 10 seconds timeout

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi connected successfully");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "WiFi connection timed out");
        return ESP_ERR_TIMEOUT;
    }

    return ESP_OK;

}