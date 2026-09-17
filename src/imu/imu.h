
#ifndef IMU_H
#define IMU_H

#include "esp_err.h"
#include "mpu6050.h"
#include <driver/i2c_master.h>

typedef struct {
  i2c_master_bus_handle_t i2c_bus_handle;
  i2c_master_dev_handle_t i2c_dev_handle;
  float accel_sens; 
  float gyro_sens;
} imu_handle_t;

typedef struct {
   uint16_t address;    
   uint32_t clock_speed;     
   accel_ranges_e accel_range;
   gyro_ranges_e gyro_range;
} imu_config_t;

/* PUBLIC */
esp_err_t imu_init(imu_handle_t* dev, const imu_config_t config);
imu_config_t imu_default_config();


/* PRIVATE */
esp_err_t imu_configure(imu_handle_t* dev, const imu_config_t config);
esp_err_t i2c_init(i2c_master_bus_handle_t* handle, uint8_t scl_num, uint8_t sda_num);

float get_accel_sensitivity(accel_ranges_e range);
float get_gyro_sensitivity(gyro_ranges_e range);


#endif
