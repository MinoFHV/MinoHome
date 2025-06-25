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
#include "dht20.h"

void app_main()
{

    // Flash NVS to store WiFi Configuration data
    nvs_flash_init();

    // Init WiFi
    wifi_init_and_connect();
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Init MQTT
    mqtt_init();

    // Init I2C
    dht20_init();

    float dht20_temp = 0.0f;
    float dht20_humid = 0.0f;

    // Testing Area
    while (1)
    {

        dht20_read_temperature_and_humidity(&dht20_temp, &dht20_humid);

        sendMQTTpayload("home/esp32/temperature", &dht20_temp, format_float);
        sendMQTTpayload("home/esp32/humidity", &dht20_humid, format_float);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}