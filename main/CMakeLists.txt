idf_component_register(
    SRCS
        "main.c"
        "./src/my_mqtt.c"
        "./src/my_wifi.c"
        "./src/i2c_init.c"
        "./src/dht20.c"
        "./src/potentiometer.c"
    PRIV_REQUIRES spi_flash mqtt esp_wifi nvs_flash driver esp_adc
    INCLUDE_DIRS "./includes"
)
