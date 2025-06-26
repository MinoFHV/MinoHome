idf_component_register(
    SRCS
        "main.c"
        "./src/led_alive.c"
        "./src/my_mqtt.c"
        "./src/my_wifi.c"
        "./src/i2c_init.c"
        "./src/dht20.c"
        "./src/bh1750.c"
        "./src/potentiometer.c"
        "./src/waveshare_tvoc_sensor.c"
    PRIV_REQUIRES spi_flash mqtt esp_wifi nvs_flash driver esp_adc
    INCLUDE_DIRS "./includes"
)
