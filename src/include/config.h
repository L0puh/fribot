#ifndef MAIN_CONFIG_H
#define MAIN_CONFIG_H 

#include <sdkconfig.h>

#define I2C_MASTER_SCL_IO  22
#define I2C_MASTER_SDA_IO  21

#define MPU6050_DEFAULT_ADDRESS 0x68
#define MPU6050_DEFAULT_CLOCK 100000 // 1MHz 

// MPU sensitivity
#define ACCEL_FULL_SCALE_RANGE 8192 
#define GYRO_FULL_SCALE_RANGE 65.5

#define BLINK_LED_PIN 2

// motors 
#define MOTOR_A_IN1 (32)
#define MOTOR_A_IN2 (33)
#define MOTOR_B_IN1 (25)
#define MOTOR_B_IN2 (26)


// wifi 
#define FRIBOT_WIFI_SSID    "FRIBOT_NET"
#define FRIBOT_WIFI_PSWD    "fribot123" // at least 8 is required
#define FRIBOT_WIFI_CHANNEL 1
#define FRIBOT_MAX_STA_CONN 4

#endif
