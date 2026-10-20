#include "imu.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "fribot.h"
#include "mpu6050.h"


esp_err_t imu_whoami_check(imu_handle_t *dev) 
{
   esp_err_t ret;
   uint8_t imu_whoami;
   ret = imu_read(dev, MPU6050_WHO_AM_I, &imu_whoami, 1);
   ESP_ERROR_CHECK(ret);
   ESP_LOGI("imu", "who am i = 0x%02X", imu_whoami);
   if (imu_whoami != MPU6050_WHO_AM_I_VAL)
   {
      ESP_LOGE("imu", "wrong falue for who am i!");
      return ESP_ERR_INVALID_RESPONSE;
   }

   return ESP_OK;
}

esp_err_t i2c_init(i2c_master_bus_handle_t* handle, uint8_t scl_num, uint8_t sda_num)
{
   i2c_master_bus_config_t i2c_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = GPIO_NUM_0,
      .scl_io_num = scl_num,
      .sda_io_num = sda_num,
      .flags.enable_internal_pullup = true,
   };

   ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, handle));

   return ESP_OK;
}

esp_err_t imu_read(imu_handle_t *dev, const uint8_t addr,
      uint8_t *buf, const uint8_t size)
{
   return i2c_master_transmit_receive(dev->i2c_dev_handle, &addr, 1, buf, size, -1);
}

esp_err_t imu_read_raw_data(imu_handle_t *dev, imu_raw_data_t *data)
{
   esp_err_t ret;
   uint8_t raw_data[6];

   if (data == NULL || dev == NULL) {
      return ESP_ERR_INVALID_ARG;
   }
  
   memset(raw_data, 0, sizeof(raw_data));
   ret = imu_read(dev, MPU6050_ACCEL_XOUT_H, raw_data, sizeof(raw_data));
   
   if (ret == ESP_OK){
      data->recv_accel = true;
      data->accel.x = (int16_t)((raw_data[0] << 8) + (raw_data[1]));
      data->accel.y = (int16_t)((raw_data[2] << 8) + (raw_data[3]));
      data->accel.z = (int16_t)((raw_data[4] << 8) + (raw_data[5]));
   }
  
   memset(raw_data, 0, sizeof(raw_data));
   ret = imu_read(dev, MPU6050_GYRO_XOUT_H, raw_data, sizeof(raw_data));
   
   if (ret == ESP_OK){
      data->recv_gyro = true;
      data->gyro.x = (int16_t)((raw_data[0] << 8) + (raw_data[1]));
      data->gyro.y = (int16_t)((raw_data[2] << 8) + (raw_data[3]));
      data->gyro.z = (int16_t)((raw_data[4] << 8) + (raw_data[5]));
   }

   return ret;
}


esp_err_t imu_write(imu_handle_t *dev, const uint8_t addr, 
      const uint8_t *buf, const uint8_t size)
{

   //TODO: 
   // check buffer overflow
   // and probably change the timeout to some value 

   uint8_t data[size+1];
   data[0] = addr;
   for (uint8_t i = 0; i < size; i++){
      data[i+1] = buf[i];
   }

   return i2c_master_transmit(dev->i2c_dev_handle, data, size+1, -1);
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
   ret = imu_write(dev, MPU6050_GYRO_CONFIG, &cfg[0], 2);
   ESP_ERROR_CHECK(ret);

   dev->gyro_sens = get_gyro_sensitivity(config.gyro_range);
   dev->accel_sens = get_accel_sensitivity(config.accel_range);

   if (config.auto_wakeup) {
      return imu_wakeup(dev);
   }

   return ESP_OK;
}

esp_err_t imu_wakeup(imu_handle_t* dev) 
{
   uint8_t value;
   esp_err_t ret;

   ret = imu_read(dev, MPU6050_PWR_MGMT_1, &value, 1);
   ESP_ERROR_CHECK(ret);

   value &= (~BIT6);

   return imu_write(dev, MPU6050_PWR_MGMT_1, &value, 1);
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
      .gyro_range = GYRO_500DPS,
      .auto_wakeup = true
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
   return 0;
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

   return 0;
}

