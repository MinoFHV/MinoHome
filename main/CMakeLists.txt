idf_component_register(
    SRCS
        "main.c"
        "./src/my_mqtt.c"
        "./src/my_wifi.c"
    PRIV_REQUIRES spi_flash mqtt esp_wifi nvs_flash
    INCLUDE_DIRS "./includes"
)
