#include "utils/led_alive.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_strip.h"


#define BLINK_GPIO          8
#define BLINK_RESOLUTION    (10 * 1000 * 1000)  // 10MHz


static led_strip_handle_t led_strip = NULL;
static const char *TAG = "LED";


void led_program_alive_task(void *pvParameters)
{

    while (1)
    {

        led_strip_set_pixel(led_strip, 0, 0, 16, 0);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(150));
        led_strip_clear(led_strip);
        vTaskDelay(pdMS_TO_TICKS(850));

    }

}

esp_err_t led_init()
{

    led_strip_config_t led_strip_config =
    {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1
    };

    led_strip_rmt_config_t led_strip_rmt_config =
    {
        .resolution_hz = BLINK_RESOLUTION,
        .flags.with_dma = false
    };

    esp_err_t ret = led_strip_new_rmt_device(&led_strip_config, &led_strip_rmt_config, &led_strip);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "led_strip_new_rmt_device failed: %s", esp_err_to_name(ret));
        return ret;
    }

    led_strip_clear(led_strip);
    return ret;

}