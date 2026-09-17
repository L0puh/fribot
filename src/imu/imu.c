#include "imu.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "fribot.h"
#include "mpu6050.h"


esp_err_t i2c_init(i2c_master_bus_handle_t* handle, uint8_t scl_num, uint8_t sda_num)
{
   i2c_master_bus_config_t i2c_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = 0,
      .scl_io_num = scl_num,
      .sda_io_num = sda_num,
      .flags.enable_internal_pullup = true,
   };

   ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, handle));

   return ESP_OK;
}

esp_err_t imu_configure(imu_handle_t* dev, const imu_config_t config)
{
   esp_err_t ret;
   i2c_device_config_t dev_config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = config.address,
      .scl_speed_hz = config.clock_speed
   };

   ret = i2c_master_bus_add_device(dev->i2c_bus_handle, &dev_config, &dev->i2c_dev_handle);
   ESP_ERROR_CHECK(ret);


   uint8_t cfg[2] = { config.gyro_range << 3, config.accel_range << 3};
   //esp_err_t imu_write(dev, MPU6050_GYRO_CONFIG, &cfg[0], 2);

   dev->gyro_sens = get_gyro_sensitivity(config.gyro_range);
   dev->accel_sens = get_accel_sensitivity(config.accel_range);

   // if (config.auto_wakeup) {
   //    return imu_wakeup(dev);
   // }

   return ESP_OK;
}

esp_err_t imu_init(imu_handle_t* dev, const imu_config_t config)
{
   if (i2c_init(&dev->i2c_bus_handle, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO) != ESP_OK) 
   {
      return ESP_FAIL;
   }

   if (imu_configure(dev, config) != ESP_OK) 
   {
      return ESP_FAIL;
   }

   return ESP_OK;
}

imu_config_t imu_default_config()
{
   imu_config_t cfg = {
      .address = MPU6050_DEFAULT_ADDRESS,
      .clock_speed = MPU6050_DEFAULT_CLOCK,
      .accel_range = ACCEL_4G,
      .gyro_range = GYRO_500DPS
   };

   return cfg;
}

float get_gyro_sensitivity(gyro_ranges_e range)
{

  switch (range) {
     case GYRO_250DPS:  return 131;
     case GYRO_500DPS:  return 65.5;
     case GYRO_1000DPS: return 32.8;
     case GYRO_2000DPS: return 16.4;
       break;
  }
}
float get_accel_sensitivity(accel_ranges_e range)
{
   switch(range)
   {
      case ACCEL_2G:  return 16384;
      case ACCEL_4G:  return 8192;
      case ACCEL_8G:  return 4096;
      case ACCEL_16G: return 2048;
        break;
   }
}

