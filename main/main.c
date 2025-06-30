// ESP-IDF & C specific includes
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include <stdio.h>

// Custom Includes + Implementations
#include "led_alive.h"

#include "my_mqtt.h"
#include "my_wifi.h"

#include "i2c_init.h"
#include "dht20.h"
#include "bh1750.h"
#include "potentiometer.h"
#include "waveshare_tvoc_sensor.h"

void app_main()
{

    // Flash NVS to store WiFi Configuration data
    nvs_flash_init();

    // Init WiFi
    wifi_init_and_connect();

    // Init MQTT
    mqtt_init();

    // Init I2C Bus
    i2c_bus_init();

    // Init Sensors
    dht20_init(); // Temperature & Humidity Sensor
    bh1750_init(); // Light Sensor
    adc_init(); // ADC Init for Potentiometer
    uart_init(); // UART Init for TVOC Sensor
    tvoc_set_active_mode(); // Setting TVOC Sensor to Active

    // Init RGB LED to signify program is alive
    led_init();

    // Create FreeRTOS tasks!
    xTaskCreate(led_program_alive_task, "led_program_alive_task", 2048, NULL, 0, NULL);
    xTaskCreate(dht20_measure_and_sendmqtt_task, "dht20_measure_and_sendmqtt_task", 4096, NULL, 5, NULL);
    xTaskCreate(bh1750_measure_and_sendmqtt_task, "bh1750_measure_and_sendmqtt_task", 4096, NULL, 5, NULL);
    xTaskCreate(tvoc_sensor_measure_and_sendmqtt_task, "tvoc_sensor_measure_and_sendmqtt_task", 4096, NULL, 5, NULL);
    xTaskCreate(adc_potentiometer_sendmqtt_task, "adc_potentiometer_sendmqtt_task", 4096, NULL, 4, NULL);

}