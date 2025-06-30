// ESP-IDF & C specific includes
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include <stdio.h>

// Wireless Protocol Modules
#include "wireless-protocol-modules/mqtt.h"
#include "wireless-protocol-modules/wifi.h"

// Wired Protocol Modules
#include "wired-protocol-modules/i2c.h"
#include "wired-protocol-modules/uart.h"

// Sensor Modules
#include "sensors/dht20.h"
#include "sensors/bh1750.h"
#include "sensors/waveshare_tvoc_sensor.h"
#include "sensors/potentiometer.h"


void freeze_program_if_init_error(const char* TAG, esp_err_t err)
{

    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "ERROR, PLEASE FIX AND THEN REBOOT");
        while (1) { vTaskDelay((pdMS_TO_TICKS(1000))); } // Freeze system for debugging

    }

}

void app_main()
{

    /*
        DEVELOPER'S NOTE: I would've set up LEDs for error states with blinking patterns to signify errors and allow for on-view debugging.
                          This is why some of my functions have esp_err_t or custom error types, but they're not being handled.
                          The reason for this is that due to illness, I didn't have the time to find my LEDs + resistors, otherwise it would've been done.
    */

    // Flash NVS to store WiFi Configuration data
    if (nvs_flash_init() != 0) freeze_program_if_init_error("NVM", ESP_FAIL);

    // Init WiFi
    freeze_program_if_init_error("WiFi", wifi_init_and_connect());

    // Init MQTT
    freeze_program_if_init_error("MQTT", mqtt_init());

    // Init I2C Bus
    freeze_program_if_init_error("I2C", i2c_master_bus_init());

    // Init Sensors
    freeze_program_if_init_error("DHT20_INIT", dht20_init()); // Temperature & Humidity Sensor
    freeze_program_if_init_error("BH1750_INIT", bh1750_init()); // Light Sensor
    freeze_program_if_init_error("ADC_INIT", adc_init()); // ADC Init for Potentiometer
    freeze_program_if_init_error("UART_INIT", uart_init()); // UART Init for TVOC Sensor TODO: Move this to after i2c_bus_init();
    freeze_program_if_init_error("TVOC_INIT", tvoc_set_active_mode()); // Setting TVOC Sensor to Active

    // FreeRTOS Sensor Tasks
    xTaskCreate(dht20_measure_and_sendmqtt_task, "dht20_measure_and_sendmqtt_task", 4096, NULL, 5, NULL);
    xTaskCreate(bh1750_measure_and_sendmqtt_task, "bh1750_measure_and_sendmqtt_task", 4096, NULL, 5, NULL);
    xTaskCreate(tvoc_sensor_measure_and_sendmqtt_task, "tvoc_sensor_measure_and_sendmqtt_task", 4096, NULL, 5, NULL);
    xTaskCreate(adc_potentiometer_sendmqtt_task, "adc_potentiometer_sendmqtt_task", 4096, NULL, 4, NULL);

}