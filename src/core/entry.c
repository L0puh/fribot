#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "fribot.h"
#include "blink.h"
#include "imu.h"
#include "mpu6050.h"


void dead_loop() {
   while(true) {}
}

void app_main(void) {
   esp_err_t ret;

   blink_setup(BLINK_LED_PIN);
   blink_setup(BLINK_DEBUG);
   imu_handle_t imu_handle;

   ret = imu_init(&imu_handle, imu_default_config());

   if (ret != ESP_OK){
      ESP_LOGE("entry", "imu failed to init: %s", esp_err_to_name(ret));
      blink_times(BLINK_DEBUG, 1000);
      dead_loop();
   }
   
   ret = imu_whoami_check(&imu_handle);
   ESP_ERROR_CHECK(ret);
   ESP_LOGE("entry", "imu init successfull!");

   blink_times(BLINK_LED_PIN, 3);
   imu_raw_data_t raw_data;
   while (true)
   {
      raw_data.recv_accel = false;
      raw_data.recv_gyro  = false;

      ret = imu_read_raw_data(&imu_handle, &raw_data);
      ESP_ERROR_CHECK(ret);

      if (raw_data.recv_accel){
         printf("accel: x = %d, y = %d, z = %d\n", (int)raw_data.accel.x, 
                  (int)raw_data.accel.y, (int)raw_data.accel.z);

      }
      if (raw_data.recv_gyro){
         printf("gyro: x = %d, y = %d, z = %d\n", (int)raw_data.gyro.x, 
                  (int)raw_data.gyro.y, (int)raw_data.gyro.z);
      }
      
      vTaskDelay(pdMS_TO_TICKS(1000));
   }
}
