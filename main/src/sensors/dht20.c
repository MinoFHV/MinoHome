// Implementation according to datasheet: https://aqicn.org/air/sensor/spec/asair-dht20.pdf

#include "sensors/dht20.h"
#include "wired-protocol-modules/i2c.h"
#include "wireless-protocol-modules/mqtt.h"
#include "utils/utils.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#define DHT20_ADDR                  0x38
#define DHT20_CRC_POLYNOMIAL        0x31
#define DHT20_SCALE_FACTOR_F        ((float)(1 << 20)) // 2^20
#define DHT20_DATA_SIZE             7
#define DHT20_MEASURE_CMD_LEN       3

#define DHT20_I2C_MASTER_FREQ_HZ    100000 // 100kHz, no minimum SCL frequency required since interrface contains completely state logic according to datasheet

#define FREERTOS_TASK_REFRESH_TIME  15000 // 15 seconds because such data changes slowly


static const char *TAG = "DHT20";
static i2c_master_dev_handle_t dht20_dev_handle = NULL;


static uint8_t dht20_crc8_check(const uint8_t *data, uint8_t len)
{

    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; ++i)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ DHT20_CRC_POLYNOMIAL : (crc << 1);
        }
    }

    return crc;

}

static esp_err_t dht20_start_measurement()
{

    uint8_t commands[DHT20_MEASURE_CMD_LEN] = {0xAC, 0x33, 0x00}; // AC = Trigger Measure, Command Parameters: 0x33 and 0x00
    esp_err_t ret = check_esp_err(i2c_master_transmit(dht20_dev_handle, commands, DHT20_MEASURE_CMD_LEN, pdMS_TO_TICKS(100)), "i2c_master_transmit", TAG);
    return ret;

}

static esp_err_t dht20_read_data(uint8_t *data, uint8_t len)
{

    esp_err_t ret = check_esp_err(i2c_master_receive(dht20_dev_handle, data, len, pdMS_TO_TICKS(100)), "i2c_master_receive", TAG);
    return ret;

}


esp_err_t dht20_init(void)
{

    i2c_device_config_t i2c_device_config =
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DHT20_ADDR,
        .scl_speed_hz = DHT20_I2C_MASTER_FREQ_HZ
    };

    ESP_LOGI(TAG, "Adding DHT20 device to I2C bus...");
    esp_err_t ret = check_esp_err(i2c_master_bus_add_device(i2c_get_master_bus_handle(), &i2c_device_config, &dht20_dev_handle), "i2c_master_bus_add_device", TAG);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "DHT20 device initialized!");
    return ESP_OK;

}

esp_err_t dht20_read_temperature_and_humidity(float *temperature, float *humidity)
{

    esp_err_t ret = dht20_start_measurement();
    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(100)); // According to datasheet, wait > 80ms

    uint8_t data[DHT20_DATA_SIZE];
    ret = check_esp_err(dht20_read_data(data, DHT20_DATA_SIZE), "dht20_read_data", TAG);
    if (ret != ESP_OK) return ret;

    if (dht20_crc8_check(data, DHT20_DATA_SIZE - 1) != data[DHT20_DATA_SIZE - 1])
    {
        ESP_LOGE(TAG, "CRC Check failed!");
        return ESP_ERR_INVALID_CRC;
    }

    // Parse data from uint32_t to float
    uint32_t raw_humidity = ((uint32_t)(data[1]) << 12) | ((uint32_t)(data[2]) << 4) | ((data[3] >> 4) & 0x0F);
    uint32_t raw_temperature = (((uint32_t)(data[3] & 0x0F)) << 16) | ((uint32_t)(data[4]) << 8) | data[5];

    *humidity = (raw_humidity * 100.0f) / DHT20_SCALE_FACTOR_F;
    *temperature = ((raw_temperature * 200.0f) / DHT20_SCALE_FACTOR_F) - 50.0f;

    return ESP_OK;

}

void dht20_measure_and_sendmqtt_task(void *pvParameters)
{

    // This is to make sure that the task always runs at a fixed interval
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(FREERTOS_TASK_REFRESH_TIME);

    SemaphoreHandle_t i2c_semaphore = i2c_get_semaphore();
    float temperature = 0.0f;
    float humidity = 0.0f;

    while (1)
    {

        // Locking mechanism to prevent I2C master bus errors during simultaneous access
        if (xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            dht20_read_temperature_and_humidity(&temperature, &humidity);
            xSemaphoreGive(i2c_semaphore);
        }

        sendMQTTpayload(MQTT_TOPIC_TEMPERATURE, &temperature, format_float);
        sendMQTTpayload(MQTT_TOPIC_HUMIDITY, &humidity, format_float);
        
        vTaskDelayUntil(&last_wake_time, interval);

    }

}