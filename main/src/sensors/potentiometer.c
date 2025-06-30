#include "sensors/potentiometer.h"
#include "wireless-protocol-modules/mqtt.h"
#include "utils/utils.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define ADC_UNIT        ADC_UNIT_1
#define ADC_ATTEN       ADC_ATTEN_DB_12     // 11 is deprecated, 12 behaves the same as ADC_ATTEN_DB_12
#define ADC_BITWIDTH    ADC_BITWIDTH_12
#define ADC_CHANNEL     ADC_CHANNEL_0

#define FREERTOS_TASK_REFRESH_TIME  200     // 200ms should be enough


static const char *TAG = "ADC";
static adc_oneshot_unit_handle_t adc_oneshot_unit_handle;

static const float max_voltage = 3.3f;
static const uint16_t max_adc_raw_value = ((uint16_t)(1 << 12)); // 12 because ADC_ATTEN is 12


esp_err_t adc_init()
{

    adc_oneshot_unit_init_cfg_t adc_oneshot_unit_init_cfg =
    {
        .unit_id = ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE    // ULP Mode = Ultra Low Power Mode, read data while MCU is in "Deep-Sleep Mode"
    };

    esp_err_t ret = check_esp_err(adc_oneshot_new_unit(&adc_oneshot_unit_init_cfg, &adc_oneshot_unit_handle), "adc_oneshot_new_unit", TAG);
    if (ret != ESP_OK) return ret;

    adc_oneshot_chan_cfg_t adc_oneshot_chan_cfg =
    {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH    // Values to read between 0 - 4095
    };

    ret = check_esp_err(adc_oneshot_config_channel(adc_oneshot_unit_handle, ADC_CHANNEL, &adc_oneshot_chan_cfg), "adc_oneshot_config_channel", TAG);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "ADC successfully configured!");
    return ESP_OK;

}

esp_err_t adc_potentiometer_read_voltage(float *voltage)
{

    int potentiometer_raw_value = 0;    // adc_oneshot_read requires int!
    esp_err_t ret = check_esp_err(adc_oneshot_read(adc_oneshot_unit_handle, ADC_CHANNEL, &potentiometer_raw_value), "adc_oneshot_read", TAG);
    if (ret != ESP_OK) return ret;

    *voltage = ( ((float) potentiometer_raw_value) / ((float) max_adc_raw_value) ) * max_voltage;
    return ESP_OK;

}

void adc_potentiometer_sendmqtt_task(void *pvParameters)
{

    // This is to make sure that the task always runs at a fixed interval
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(FREERTOS_TASK_REFRESH_TIME);

    float potentiometer_voltage = 0.0f;

    while (1)
    {

        adc_potentiometer_read_voltage(&potentiometer_voltage);
        sendMQTTpayload("home/esp32/poti_voltage", &potentiometer_voltage, format_float);

        vTaskDelayUntil(&last_wake_time, interval);

    }

}