#include "blink.h"
#include "esp_err.h"

static esp_err_t blink_led(uint8_t pin, bool led_state)
{
   return gpio_set_level(pin, led_state);
}

esp_err_t blink_setup(const uint8_t pin) {
   gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&io_conf);
}

void blink_times(const uint8_t pin, const uint32_t count)
{
   uint32_t cnt = count;
   while (cnt--) 
   {
      ESP_ERROR_CHECK(blink_led(pin, true));
      vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
      ESP_ERROR_CHECK(blink_led(pin, false));
      vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
   }
}
