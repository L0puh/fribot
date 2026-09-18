
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
   bool auto_wakeup;
} imu_config_t;

typedef struct {

   struct {
      int16_t x, y, z;
   } accel;
   
   struct {
      int16_t x, y, z;
   } gyro;

   bool recv_gyro;
   bool recv_accel;
} imu_raw_data_t;

/* PUBLIC */
esp_err_t imu_init(imu_handle_t* dev, const imu_config_t config);
imu_config_t imu_default_config();
esp_err_t imu_wakeup(imu_handle_t* dev);
esp_err_t imu_whoami_check(imu_handle_t *dev);
esp_err_t imu_read(imu_handle_t *dev, const uint8_t addr, uint8_t *buf, const uint8_t size);
esp_err_t imu_write(imu_handle_t *dev, const uint8_t addr, const uint8_t *buf, const uint8_t size);
esp_err_t imu_read_raw_data(imu_handle_t *dev, imu_raw_data_t *data);

/* PRIVATE */
esp_err_t imu_configure(imu_handle_t* dev, const imu_config_t config);
esp_err_t i2c_init(i2c_master_bus_handle_t* handle, uint8_t scl_num, uint8_t sda_num);

float get_accel_sensitivity(accel_ranges_e range);
float get_gyro_sensitivity(gyro_ranges_e range);

#endif
