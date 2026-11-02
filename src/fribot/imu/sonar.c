#include "sonar.h"
#include "config.h"
#include "driver/rmt_common.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>
#include <esp_log.h>

static sonar_t sonar_g;

static bool IRAM_ATTR rmt_rx_done_callback(
    rmt_channel_handle_t chan,
    const rmt_rx_done_event_data_t *edata,
    void *user_data
) {
    for (size_t i = 0; i < edata->num_symbols; i++) {
        rmt_symbol_word_t sym = edata->received_symbols[i];

        if (sym.level0 == 1 && sym.duration0 > 0) {
            sonar_g.echo_time_us = sym.duration0;
            sonar_g.measurement_done = true;
            break;
        }

        if (sym.level1 == 1 && sym.duration1 > 0) {
            sonar_g.echo_time_us = sym.duration1;
            sonar_g.measurement_done = true;
            break;
        }
    }

    return false;
}

void trigger_pulse(void)
{
   gpio_set_level(SONAR_TRIG_PIN, 1);
   esp_rom_delay_us(TRIGGER_US);
   gpio_set_level(SONAR_TRIG_PIN, 0);
}

esp_err_t sonar_init(void)
{

   esp_err_t ret;
   gpio_config_t trig_cfg = {
      .pin_bit_mask = 1ULL << SONAR_TRIG_PIN,
      .mode = GPIO_MODE_OUTPUT, 
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
   };

   ret = gpio_config(&trig_cfg);
   ESP_ERROR_CHECK(ret);

   ret = gpio_set_level(SONAR_TRIG_PIN, 0);
   ESP_ERROR_CHECK(ret);

   rmt_rx_channel_config_t rmt_cfg = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = RMT_RESOLUTION_HZ,
      .mem_block_symbols = 64,
      .gpio_num = SONAR_ECHO_PIN,
      .flags.invert_in = false, 
      .flags.with_dma = false, 
   };

   ret = rmt_new_rx_channel(&rmt_cfg, &sonar_g.rmt_channel_g);
   ESP_ERROR_CHECK(ret);

   rmt_rx_event_callbacks_t callbacks = {
      .on_recv_done = rmt_rx_done_callback,
   };

   ret = rmt_rx_register_event_callbacks(sonar_g.rmt_channel_g,
         &callbacks, NULL);
   ESP_ERROR_CHECK(ret); 

   ret = rmt_enable(sonar_g.rmt_channel_g);
   ESP_ERROR_CHECK(ret);
   ESP_LOGI("sonar", "init sonar is done");
   return ret;
}

float sonar_measure_cm(void)
{
   esp_err_t ret;
   sonar_g.measurement_done = false;
   sonar_g.echo_time_us = 0;

   rmt_receive_config_t recv_cfg = {
      .signal_range_max_ns = 30000000, //30ms
      .signal_range_min_ns = 1000, //1us
   };

   ret = rmt_receive(sonar_g.rmt_channel_g, sonar_g.rmt_symbol_g, 
         sizeof(sonar_g.rmt_symbol_g), &recv_cfg);

   if (ret != ESP_OK)
   {
      ESP_LOGE("sonar", "rmt recv failed %s", esp_err_to_name(ret));
      return -1.0f;
   }
   trigger_pulse();

   for (int i = 0; i < 7; i++){
      if (sonar_g.measurement_done){
         break;
      }

      vTaskDelay(pdMS_TO_TICKS(10));
   }

   if (!sonar_g.measurement_done){
      ESP_LOGW("sonar", "measuremnt timeout");

      ret = rmt_disable(sonar_g.rmt_channel_g);

      if (ret != ESP_OK)
      {
         ESP_LOGE("sonar", "rmt disable failed %s", esp_err_to_name(ret));
         return -1.0f;
      }

      ret = rmt_enable(sonar_g.rmt_channel_g);

      if (ret != ESP_OK)
      {
         ESP_LOGE("sonar", "rmt enable failed %s", esp_err_to_name(ret));
      }

      return -1.0f;
   }
   
   float dist = ((float) sonar_g.echo_time_us * 0.0343f) / 2.0f;
   return dist;
}
