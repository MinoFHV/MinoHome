idf_component_register(
    SRCS
        "main.c"
        "./src/utils/utils.c"
        "./src/wireless-protocol-modules/mqtt.c"
        "./src/wireless-protocol-modules/wifi.c"
        "./src/wired-protocol-modules/i2c.c"
        "./src/wired-protocol-modules/uart.c"
        "./src/sensors/dht20.c"
        "./src/sensors/bh1750.c"
        "./src/sensors/waveshare_tvoc_sensor.c"
        "./src/sensors/potentiometer.c"
    PRIV_REQUIRES spi_flash mqtt esp_wifi nvs_flash driver esp_adc
    INCLUDE_DIRS "./includes"
)
