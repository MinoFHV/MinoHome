// Implementation according to datasheet: https://www.waveshare.com/wiki/TVOC_Sensor
// The TVOC Sensor requires some warmup time up until 120 seconds per specification, so please wait until the data arrives on Home Assistant

#include "sensors/waveshare_tvoc_sensor.h"
#include "wireless-protocol-modules/mqtt.h"
#include "wired-protocol-modules/uart.h"

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define TVOC_UART_PORT_NUM              UART_NUM_0

#define TVOC_FRAME_HEADER               0xFE
#define TVOC_FRAME_FOOTER               0x16
#define TVOC_FRAME_SIZE                 11
#define TVOC_ACTIVE_MODE_CMD_LEN        9
#define TVOC_WARMUP_WAIT_TIME_MS        120000  // 120 seconds

#define FREERTOS_TASK_REFRESH_TIME      10000   // Usually, you'd do 30-60 seconds, but this is just better for the showcase :3


static const char *TAG = "TVOC";
static const uint8_t TVOC_ACTIVE_MODE_CMD[TVOC_ACTIVE_MODE_CMD_LEN] = {0xFE, 0x00, 0x78, 0x40, 0x00, 0x00, 0x00, 0x00, 0xB8};


esp_err_t tvoc_set_active_mode()
{

    int bytes_written = uart_write_bytes(TVOC_UART_PORT_NUM, (const char *)TVOC_ACTIVE_MODE_CMD, TVOC_ACTIVE_MODE_CMD_LEN);
    if (bytes_written != TVOC_ACTIVE_MODE_CMD_LEN)
    {
        ESP_LOGE(TAG, "Failed to send active mode command");
        return ESP_FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(120)); // 120ms required
    ESP_LOGI(TAG, "Active mode command sent");
    return ESP_OK;

}

tvoc_sensor_uart_status_t tvoc_read_co2_ch2o_tvoc_airquality(uint8_t *air_quality, uint16_t *co2, uint16_t *ch2o, float *tvoc)
{

    uint8_t response_frame[TVOC_FRAME_SIZE];
    uint8_t frame_idx = 0;
    int32_t bytes_read;

    // Read byte by byte of frame
    while (frame_idx < TVOC_FRAME_SIZE)
    {

        bytes_read = uart_read_bytes(TVOC_UART_PORT_NUM, &response_frame[frame_idx], 1, pdMS_TO_TICKS(1000));
        if (bytes_read == 1)
        {
            if ((frame_idx == 0) && (response_frame[0] != TVOC_FRAME_HEADER)) continue; // This is here to wait for the header (because we send no command when to start, we just read)
            ++frame_idx;
        }
        else
        {
            ESP_LOGE(TAG, "Error reading UART bytes: Sensor Error Timeout");
            return SENSOR_ERROR_TIMEOUT;
        }

    }

    // Validate frame itself (0 = 0xFE, 10 = 0x16)
    if ((response_frame[0] != TVOC_FRAME_HEADER) || (response_frame[TVOC_FRAME_SIZE - 1] != TVOC_FRAME_FOOTER))
    {
        ESP_LOGE(TAG, "Validation of frame erroneous: Header or Footer wrong");
        return SENSOR_ERROR_HEADER_FOOTER;
    }

    // Validate Checksum
    uint8_t checksum = 0;
    for (uint8_t i = 3; i < TVOC_FRAME_SIZE - 2; ++i)
    {
        checksum += response_frame[i];
    }
    if (checksum != response_frame[TVOC_FRAME_SIZE - 2])
    {
        ESP_LOGE(TAG, "Checksum failed");
        return SENSOR_ERROR_CHECKSUM;
    }

    // Extract values (little-endian)
    *air_quality = response_frame[1];
    *co2 = (response_frame[3] << 8) | response_frame[4];
    *ch2o = (response_frame[5] << 8) | response_frame[6];
    *tvoc = ((response_frame[7] << 8) | response_frame[8]) / 1000.0f;

    return SENSOR_OK;

}

void tvoc_sensor_measure_and_sendmqtt_task(void *pvParameters)
{

    // Initial 120s warmup delay
    ESP_LOGI(TAG, "TVOC Sensor warming up for 120 seconds...");
    vTaskDelay(pdMS_TO_TICKS(TVOC_WARMUP_WAIT_TIME_MS));
    ESP_LOGI(TAG, "Warmup done. Starting regular measurements.");

    tvoc_set_active_mode();

    // This is to make sure that the task always runs at a fixed interval
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(FREERTOS_TASK_REFRESH_TIME);

    uint8_t air_quality = 0;
    uint16_t co2 = 0;
    uint16_t ch2o = 0;
    float tvoc = 0;

    while (1)
    {

        if (tvoc_read_co2_ch2o_tvoc_airquality(&air_quality, &co2, &ch2o, &tvoc) == SENSOR_OK)
        {

            sendMQTTpayload(MQTT_TOPIC_CO2, &co2, format_uint16);
            sendMQTTpayload(MQTT_TOPIC_CH2O, &ch2o, format_uint16);
            sendMQTTpayload(MQTT_TOPIC_TVOC, &tvoc, format_float);
            sendMQTTpayload(MQTT_TOPIC_AIRQUALITY, &air_quality, format_uint8);

        };

        vTaskDelayUntil(&last_wake_time, interval);

    }

}