// ESP-IDF & C specific includes
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include <stdio.h>

// Custom Includes + Implementations
#include "my_mqtt.h"
#include "my_wifi.h"


void app_main()
{

    // Flash NVS to store WiFi Configuration data
    nvs_flash_init();

    // Init WiFi
    wifi_init_and_connect();
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Init MQTT
    mqtt_init();

    // Testing Area
    

    while (1)
    {
        uint32_t r = esp_random();
        float scaled = (float)r / UINT32_MAX;
        float fakeTemp = 20.0f + scaled * (25.0f - 20.0f);

        sendMQTTpayload("home/esp32/temperature", &fakeTemp, format_float);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}