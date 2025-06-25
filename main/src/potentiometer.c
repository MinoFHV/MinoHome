#include "potentiometer.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"


#define ADC_UNIT        ADC_UNIT_1
#define ADC_ATTEN       ADC_ATTEN_DB_12     // 11 is deprecated, 12 behaves the same as ADC_ATTEN_DB_12
#define ADC_BITWIDTH    ADC_BITWIDTH_12
#define ADC_CHANNEL     ADC_CHANNEL_0


static char *TAG = "ADC";
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

    esp_err_t ret = adc_oneshot_new_unit(&adc_oneshot_unit_init_cfg, &adc_oneshot_unit_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error on adc_oneshot_new_unit: %s", esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_chan_cfg_t adc_oneshot_chan_cfg =
    {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH    // Values to read between 0 - 4095
    };

    ret = adc_oneshot_config_channel(adc_oneshot_unit_handle, ADC_CHANNEL, &adc_oneshot_chan_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error on adc_oneshot_config_channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "ADC successfully configured!");
    return ESP_OK;

}

esp_err_t adc_potentiometer_read_voltage(float *voltage)
{

    int potentiometer_raw_value = 0;    // adc_oneshot_read requires int!
    esp_err_t ret = adc_oneshot_read(adc_oneshot_unit_handle, ADC_CHANNEL, &potentiometer_raw_value);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error on adc_oneshot_read: %s", esp_err_to_name(ret));
        return ret;
    }

    *voltage = ( ((float) potentiometer_raw_value) / ((float) max_adc_raw_value) ) * max_voltage;
    return ESP_OK;

}