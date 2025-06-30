#include "wired-protocol-modules/i2c_init.h"

#include "esp_log.h"


#define I2C_GPIO_SCL    9
#define I2C_GPIO_SDA    7

#define I2C_BUS_PORT    0

static const char* TAG = "I2C";
static i2c_master_bus_handle_t i2c_master_bus_handle = NULL;
static SemaphoreHandle_t i2c_semaphore = NULL;

esp_err_t i2c_bus_init()
{

    // Init i2c master bus
    i2c_master_bus_config_t i2c_master_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_BUS_PORT,
        .scl_io_num = I2C_GPIO_SCL,
        .sda_io_num = I2C_GPIO_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_LOGI(TAG, "Initializing I2C bus...");
    esp_err_t ret = i2c_new_master_bus(&i2c_master_bus_config, &i2c_master_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create i2c semaphore
    i2c_semaphore = xSemaphoreCreateMutex();
    if (i2c_semaphore == NULL)
    {
        ESP_LOGE(TAG, "xSemaphoreCreateMutex failed!");
        return ESP_FAIL;
    }

    return ESP_OK;

}

i2c_master_bus_handle_t get_i2c_master_bus_handle()
{
    return i2c_master_bus_handle;
}

SemaphoreHandle_t get_i2c_semaphore()
{
    return i2c_semaphore;
}