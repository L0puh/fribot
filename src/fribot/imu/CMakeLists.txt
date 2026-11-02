set(IMU_SRCS
   imu.c
   sonar.c
)

idf_component_register(
    SRCS ${IMU_SRCS}
    INCLUDE_DIRS 
        "." 
        "../../include"
    REQUIRES 
        freertos
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_rmt 
        esp_timer
)

