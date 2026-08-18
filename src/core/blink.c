#include "fribot.h"

static void blink_led(bool led_state)
{
   gpio_set_level(BLINK_GPIO, led_state);
}

void blink_setup(void) {
   gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BLINK_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
}

void blink_times(size_t count) 
{
   while (count--) 
   {
      blink_led(true);
      vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
      blink_led(false);
      vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
   }
}
