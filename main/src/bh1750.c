// Implementation according to datasheet: https://www.mouser.com/datasheet/2/348/bh1750fvi-e-186247.pdf?srsltid=AfmBOopwez9pQj3_ZB2tl0Rf4PjhdBuStoNNZ-NviN33YV-MBm9vYDA8

#include "bh1750.h"
#include "i2c_init.h"
#include "my_mqtt.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#define BH1750_ADDR                  0x5C    // This requires VCC >= 0.7V on ADDR pin, otherwise it'S 0x23 if VCC <= 0.3V
#define BH1750_CMD_CONT_HIGH_RES     0x10    // 1 lx resolution, continuous measurement (requires 120ms measurement time)
#define BH1750_DATA_SIZE             2

#define BH1750_I2C_FREQ_HZ           100000

#define FREERTOS_TASK_REFRESH_TIME   1000   // 1s, since Light can change quite rapidly in a room


static const char *TAG = "BH1750";
static i2c_master_dev_handle_t bh1750_dev_handle = NULL;

esp_err_t bh1750_init()
{

    i2c_device_config_t i2c_device_config =
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BH1750_ADDR,
        .scl_speed_hz = BH1750_I2C_FREQ_HZ
    };

    ESP_LOGI(TAG, "Adding BH1750 device to I2C bus...");
    esp_err_t ret = i2c_master_bus_add_device(get_i2c_master_bus_handle(), &i2c_device_config, &bh1750_dev_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_bus_add_device failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Send lux resolution command...");
    uint8_t cmd = BH1750_CMD_CONT_HIGH_RES; // #define BH1750_CMD_CONT_HIGH_RES results in int, so it needs to be cast as variable, since command needs to be a pointer
    ret = i2c_master_transmit(bh1750_dev_handle, &cmd, 1, pdMS_TO_TICKS(100));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_transmit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "BH1750 device initialized!");
    return ESP_OK;

}

esp_err_t bh1750_read_lux(float *lux)
{

    uint8_t data[BH1750_DATA_SIZE];
    esp_err_t ret = i2c_master_receive(bh1750_dev_handle, data, BH1750_DATA_SIZE, pdMS_TO_TICKS(100));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read lux: %s", esp_err_to_name(ret));
        return ret;
    }

    // The raw value represents light in lux with 1.2 lx per unit
    uint16_t raw_lux = ((uint16_t) data[0] << 8) | data[1];
    *lux = raw_lux / 1.2f;

    return ESP_OK;

}

void bh1750_measure_and_sendmqtt_task(void *pvParameters)
{

    // This is to make sure that the task always runs at a fixed interval
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(FREERTOS_TASK_REFRESH_TIME);

    SemaphoreHandle_t i2c_semaphore = get_i2c_semaphore();
    float lux = 0.0f;

    while (1)
    {

        // Locking mechanism to prevent I2C master bus errors during simultaneous access
        if (xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100) == pdTRUE))
        {
            bh1750_read_lux(&lux);
            xSemaphoreGive(i2c_semaphore);
        }

        sendMQTTpayload("home/esp32/lightsensor", &lux, format_float);
        
        vTaskDelayUntil(&last_wake_time, interval);

    }

}