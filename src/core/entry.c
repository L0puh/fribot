#include "config.h"
#include "console.h"
#include "esp_err.h"
#include "esp_log.h"
#include "fribot.h"
#include "blink.h"
#include "imu.h"
#include "mpu6050.h"
#include "motors.h"
#include "cmd.h"
#include "context.h"

static int dbg_do_toggle(int argc, char **argv)
{
   const char* dbg_commands[] = {
      [0] = "motor_a",
      [1] = "motor_b",
      [2] = "imu"
   };

   if (argc != 2) {
      printf("no argument is provided, see help\n");
      return 1;
   }
   
   for (uint8_t i = 0; i < 3; i++)
   {
      if (strcmp(argv[1], dbg_commands[i]) == 0)
      {
         if (i < 2){
            bool t = ctx_g.motors[i].enabled;
            ctx_g.motors[i].enabled = !t;
            printf("!> turn %s motor %s\n", t ? "OFF": "ON", i==0 ? "A": "B");
         } else {
            printf("!> turn %s IMU\n",  ctx_g.imu_handle.enabled ? "OFF": "ON");
            ctx_g.imu_handle.enabled = !ctx_g.imu_handle.enabled;
         }
         return 0;
      }
   }
   printf("wrong argument: %s\n", argv[1]);
   return 1;
}

void dead_loop() {
   while(true) {}
}

esp_err_t setup(void)
{
   esp_err_t ret;

   blink_setup(BLINK_LED_PIN);
   blink_setup(BLINK_DEBUG);
   
   ret = imu_init(&ctx_g.imu_handle, imu_default_config());

   if (ret != ESP_OK){
      ESP_LOGE("entry", "imu failed to init: %s", esp_err_to_name(ret));
      blink_times(BLINK_DEBUG, 1000);
      dead_loop();
   }

   ret = imu_whoami_check(&ctx_g.imu_handle);
   ESP_ERROR_CHECK(ret);

   setup_motors(&ctx_g.motors[0], &ctx_g.motors[1]);
   xTaskCreate(heartbeat, "heartbeat", 2048, NULL, 1, NULL);
   ESP_LOGI("entry", "SETUP OK!");
   return ret;
}

// debug version to test the components
void debug_loop(void)
{
   esp_err_t ret;
   imu_raw_data_t data;

   ret = setup_console(&dbg_do_toggle);
   ESP_ERROR_CHECK(ret);
   ESP_LOGI("entry", "ready to start the loop");

   while (true)
   {

      if (ctx_g.motors[0].enabled)
      {
         motor_forward(&ctx_g.motors[0], 150);
         motor_stop(&ctx_g.motors[0]);
      }

      if (ctx_g.motors[1].enabled)
      {
         motor_forward(&ctx_g.motors[1], 150);
      } else {
         motor_stop(&ctx_g.motors[1]);
      }

      if (ctx_g.imu_handle.enabled)
      {
         ctx_g.raw_data.recv_accel = false, ctx_g.raw_data.recv_gyro = false;

         ret = imu_read_raw_data(&ctx_g.imu_handle, &ctx_g.raw_data);
         ESP_ERROR_CHECK(ret);

         data = ctx_g.raw_data;

         if (data.recv_accel){
            printf("accel: x = %d, y = %d, z = %d\n", (int)data.accel.x, 
                  (int)data.accel.y, (int)data.accel.z);
         }
         if (data.recv_gyro){
            printf("gyro: x = %d, y = %d, z = %d\n", (int)data.gyro.x, 
                  (int)data.gyro.y, (int)data.gyro.z);
         }
      }


      vTaskDelay(pdMS_TO_TICKS(1000));
   }
}

void loop(void)
{
   esp_err_t ret;
   imu_raw_data_t data;
   while (true)
   {
      ctx_g.raw_data.recv_accel = false, ctx_g.raw_data.recv_gyro = false;

      ret = imu_read_raw_data(&ctx_g.imu_handle, &ctx_g.raw_data);
      ESP_ERROR_CHECK(ret);

      data = ctx_g.raw_data;

      if (data.recv_accel){
         printf("accel: x = %d, y = %d, z = %d\n", (int)data.accel.x, 
               (int)data.accel.y, (int)data.accel.z);

      }
      if (data.recv_gyro){
         printf("gyro: x = %d, y = %d, z = %d\n", (int)data.gyro.x, 
               (int)data.gyro.y, (int)data.gyro.z);
      }

      vTaskDelay(pdMS_TO_TICKS(1000));
   }
}

void app_main(void) {
   setup();
   debug_loop();
}
