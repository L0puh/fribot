#include "menu.h"
#include "config.h"

#include <esp_err.h>
#include <driver/gpio.h>
#include <esp_timer.h>

static menu_t menu;

esp_err_t button_init(void)
{
   gpio_config_t cfg = {
      .pin_bit_mask = (1ULL << BUTTON_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
   };

   esp_err_t ret = gpio_config(&cfg);
   ESP_ERROR_CHECK(ret);

   menu.button.is_pressed = (gpio_get_level(BUTTON_PIN) == 0);
   menu.button.last_reading = menu.button.is_pressed;

   return ret;
}

void button_update(void)
{
   bool reading = (gpio_get_level(BUTTON_PIN) == 0);
   int64_t now = esp_timer_get_time() / 1000;

   menu.button.press_event = false;
   menu.button.release_event = false;

   if (reading != menu.button.last_reading)
   {
      menu.button.last_reading = now;
      menu.button.last_reading = reading;
   }

   if ((now - menu.button.last_change_ms) >= DEBOUNCH_TIME_MS &&
         reading != menu.button.is_pressed)
   {
      menu.button.is_pressed = reading;

      if (reading)
      {
         menu.button.press_event = true;
      } else {

         menu.button.release_event = true;
      }
   }
}

esp_err_t menu_update()
{
   button_update();

   return ESP_OK;
}

esp_err_t menu_init(void)
{
   esp_err_t ret; 
   ret = button_init();

   return ret;
}

menu_t menu_get_state(void)
{
   return menu;
}
