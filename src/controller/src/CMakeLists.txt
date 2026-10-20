set(CORE_SRCS 
   wifi.c 
   control.c 
   entry.c 
)

idf_component_register(
    SRCS ${CORE_SRCS}
    INCLUDE_DIRS 
        "." 
        "../../include/"
    REQUIRES 
        freertos
        esp_driver_gpio
        esp_driver_i2c
        nvs_flash
        esp_wifi 
)
