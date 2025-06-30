#include "wired-protocol-modules/uart.h"
#include "utils/utils.h"

#include "driver/uart.h"


#define UART_PORT_NUM       UART_NUM_0      
#define UART_BAUD_RATE      115200
#define UART_TX_PIN         20
#define UART_RX_PIN         21
#define UART_BUF_SIZE       128     // ToDo: Change ot 16 or 32 if needed, please test!


static const char *TAG = "UART";


esp_err_t uart_init()
{

    uart_config_t uart_config =
    {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    esp_err_t ret = ESP_OK;

    ret = check_esp_err(uart_param_config(UART_PORT_NUM, &uart_config), "uart_param_config", TAG);
    if (ret != ESP_OK) return ret;

    ret = check_esp_err(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE), "uart_set_pin", TAG);
    if (ret != ESP_OK) return ret;

    ret = check_esp_err(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0), "uart_driver_install", TAG);
    if (ret != ESP_OK) return ret;

    // around 200ms time needed for hardware to initialize UART
    vTaskDelay(pdMS_TO_TICKS(200));
    return ESP_OK;

}