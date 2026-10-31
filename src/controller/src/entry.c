
#include "esp_err.h"
#include "joystick.h"
#include "wifi.h"
#include "menu.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>

void setup(void)
{
   esp_err_t ret = nvs_flash_init();
   if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
   }
   ESP_ERROR_CHECK(ret);

   ret = wifi_station_init();
   if (ret != ESP_OK) {
      ESP_LOGI("entry", "couldn't connect to fribot, entering 5m sleep");
      vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));   
   }

   ret = joystick_init();
   ret = menu_init();
}

void loop(void)
{
   //FIXME: run joystick & menu as separate tasks 
   while(1){
      joystick_update();
      menu_update();
      
      if (menu_get_state().button.is_pressed) {
         pos_t p = joystick_get_pos();
         ESP_LOGI("entry", "current pos: x=%d y=%d", p.i.x, p.i.y);
      }

      vTaskDelay(pdMS_TO_TICKS(10));
   }
}

void app_main(void) 
{
   setup();
   loop();
}
