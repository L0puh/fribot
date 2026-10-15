set(RX_SRCS 
   rx.c 
   cmd.c 
   console.c
)

idf_component_register(
    SRCS ${RX_SRCS}
    INCLUDE_DIRS 
        "." 
        "../../include"
    REQUIRES 
        freertos
        esp_driver_gpio
        esp_driver_i2c
        console
)
