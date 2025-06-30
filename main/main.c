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
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Init MQTT
    mqtt_init();

    // Init I2C Bus
    i2c_bus_init();

    // Init DHT20
    dht20_init();

    // Init BH1750
    bh1750_init();

    // Init ADC
    adc_init();

    // Init UART Waveshark TVOC Sensor
    uart_init();
    tvoc_set_active_mode();

    // Init RGB LED to signify program is alive
    led_init();
    xTaskCreate(led_program_alive_task, "led_program_alive_task", 2048, NULL, 0, NULL);

    float dht20_temp = 0.0f;
    float dht20_humid = 0.0f;

    float bh1750_lux = 0.0f;

    float potentiometer_voltage = 0.0f;

    uint8_t air_quality = 0;
    uint16_t co2 = 0;
    uint16_t ch2o = 0;
    float tvoc = 0;

    // Testing Area
    while (1)
    {

        // ToDo: use FreeRTOS with Semaphores or Queues to avoid I2C Master Bus to be busy when two devices access at the same time

        dht20_read_temperature_and_humidity(&dht20_temp, &dht20_humid);
        bh1750_read_lux(&bh1750_lux);
        adc_potentiometer_read_voltage(&potentiometer_voltage);
        read_co2_ch2o_tvoc_airquality(&air_quality, &co2, &ch2o, &tvoc);

        sendMQTTpayload("home/esp32/temperature", &dht20_temp, format_float);
        sendMQTTpayload("home/esp32/humidity", &dht20_humid, format_float);
        sendMQTTpayload("home/esp32/poti_voltage", &potentiometer_voltage, format_float);
        sendMQTTpayload("home/esp32/lightsensor", &bh1750_lux, format_float);
        sendMQTTpayload("home/esp32/carbondioxide", &co2, format_uint16);
        sendMQTTpayload("home/esp32/formaldehyde", &ch2o, format_uint16);
        sendMQTTpayload("home/esp32/tvoc", &tvoc, format_float);
        sendMQTTpayload("home/esp32/airquality", &air_quality, format_uint8);

        vTaskDelay(pdMS_TO_TICKS(500));
        
    }

}