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
#include "wifi.h"
#include "rx.h"

#include <nvs_flash.h>
#include <stdint.h>
#include <esp_timer.h>
#include <string.h>

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


   // nvs setup for wifi
   ret = nvs_flash_init();
   if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
   }
   ESP_ERROR_CHECK(ret);

   ESP_LOGI("entry", "nvs init ok");

   wifi_init_softap();
   udp_server_init();
   
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

void loop(void)
{
   // esp_err_t ret;
   // imu_raw_data_t data;
   bool controller_connected = false;

#ifdef DEBUG_CONSOLE
   ret = setup_console(&dbg_do_toggle);
#else 
   xTaskCreate(rx_task, "rx task", 4096, NULL, 5, NULL);
#endif 
  // ESP_ERROR_CHECK(ret);
   ESP_LOGI("entry", "ready to start the loop");

   ctx_g.motors[0].enabled = false;
   ctx_g.motors[1].enabled = false;
   
   int64_t last_time = -1;
   while (true)
   {
      if (!controller_connected ){
         controller_connected = wifi_is_station_connected();
         ESP_LOGI("entry", "controller is %sconnected", controller_connected ? "": "dis");
      } else {
         ctx_g.raw_data.recv_accel = false, ctx_g.raw_data.recv_gyro = false;

         //ret = imu_read_raw_data(&ctx_g.imu_handle, &ctx_g.raw_data);
         //ESP_ERROR_CHECK(ret);


         ////TODO: handle imu balance 
         //data = ctx_g.raw_data;
         //if (data.recv_accel){
         //   printf("accel: x = %d, y = %d, z = %d\n", (int)data.accel.x, 
         //         (int)data.accel.y, (int)data.accel.z);

         //}
         //if (data.recv_gyro){
         //   printf("gyro: x = %d, y = %d, z = %d\n", (int)data.gyro.x, 
         //         (int)data.gyro.y, (int)data.gyro.z);
         //}

         // debug version for now

         rx_data_t rx_data = rx_get_data();
         ESP_LOGI("entry", "reading rx data: %lld ms / %lld ms", rx_data.last_ms, last_time);
         if (rx_data.last_ms != last_time)
         {
            last_time = rx_data.last_ms;
            motor_backward(&ctx_g.motors[0], 165);
            motor_backward(&ctx_g.motors[1], 165);
         } else {
            motor_stop(&ctx_g.motors[0]);
            motor_stop(&ctx_g.motors[1]);
         }
      }
      vTaskDelay(pdMS_TO_TICKS(100));
   }
}

void app_main(void) {
   setup();
   loop();
}
