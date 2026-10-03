set(MOTORS_SRCS 
   motors.c
   balance.c
)

idf_component_register(
     SRCS ${MOTORS_SRCS}
    INCLUDE_DIRS 
        "." 
        "../../include"
    REQUIRES 
        freertos
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_mcpwm
        imu
)
