#include "joystick.h"
#include "config.h"

#include <esp_err.h>
#include <nvs.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>
#include <math.h>

static adc_oneshot_unit_handle_t adc_handle_g;
static calib_t calib_data_g;
static calib_state_t calib_state_g;
static pos_t pos_g;


// NVS 
esp_err_t calibration_load(void)
{
   esp_err_t ret;
   nvs_handle_t nvs;
   size_t size;

   ret = nvs_open("joystick", NVS_READONLY, &nvs);
   if (ret != ESP_OK)
      return ret;

   size = sizeof(calib_data_g);
   ret = nvs_get_blob(nvs, "calibration", &calib_data_g, &size);
   
   nvs_close(nvs);

   if (ret != ESP_OK || size != sizeof(calib_data_g)){
      return ESP_ERR_NOT_FOUND;
   }

   return calib_data_g.version != 1 ? ESP_ERR_INVALID_VERSION: ESP_OK;
}

esp_err_t calibration_save(void) 
{
   nvs_handle_t nvs;
   esp_err_t ret;
   ret = nvs_open("joystick", NVS_READWRITE, &nvs);
   ESP_ERROR_CHECK(ret);
   ret = nvs_set_blob(nvs, "calibration", &calib_data_g, sizeof(calib_data_g));
   if (ret == ESP_OK){
      nvs_commit(nvs);
   }
   nvs_close(nvs);
   return ret;
}

// ADC 

pos_t joystick_read_raw_adc(void) {
   pos_t pos;
   esp_err_t ret;
   pos.i.x = 0; pos.i.y = 0;

   ret = adc_oneshot_read(adc_handle_g, JOY_X_CHANNEL, &pos.i.x);
   ESP_ERROR_CHECK(ret);
   
   ret = adc_oneshot_read(adc_handle_g, JOY_X_CHANNEL, &pos.i.y);
   ESP_ERROR_CHECK(ret);

   return pos;
}

void joystick_start_calibration(void)
{
   ESP_LOGI("JT", "calibration starts, release joystick");
   calib_state_g.running = true;
   calib_state_g.count = 0;
   calib_state_g.sum_x = 0;
   calib_state_g.sum_y = 0;
   pos_g.f.x = 0; pos_g.f.y = 0;
}


void joystick_update_calibration(void)
{
   pos_t pos = joystick_read_raw_adc();

   calib_state_g.sum_x += pos.i.x;
   calib_state_g.sum_y += pos.i.y;
   calib_state_g.count++;

   if (calib_state_g.count < CALIB_SAMPLES) 
      return;

   calib_data_g.center.x = calib_state_g.sum_x / CALIB_SAMPLES;
   calib_data_g.center.y = calib_state_g.sum_y / CALIB_SAMPLES;


   calib_data_g.min.x = calib_data_g.center.x;
   calib_data_g.max.x = calib_data_g.center.x;
   calib_data_g.min.y = calib_data_g.center.y;
   calib_data_g.max.y = calib_data_g.center.y;

   calib_data_g.version = 1;

   calibration_save();

   calib_state_g.running = false; 

   ESP_LOGI("JT", "calibration dome: center = (%d, %d)", 
                                    calib_data_g.center.x, 
                                    calib_data_g.center.y);
}


float normalize_axis(int raw, int center, int min, int max)
{
   float v = 0.0f;
   if (raw >= center)
   {
      if (max - center < MIN_VALID_RANGE){
         return v;
      }

      v = (float)(raw - center) / (float)(max - center);
      return fminf(v, 1.0f);
   }

   if (center - min < MIN_VALID_RANGE) {
      return v;
   }

   v = (float)(raw - center) / (float)(center - min);
   return fmaxf(v, -1.0f);
}

float deadzone(float v)
{
   float magnitude = fabs(v);
   if (magnitude < DEADZONE) 
      return 0.0f;
   magnitude = (magnitude - DEADZONE) / (1.0f - DEADZONE);

   return copysignf(magnitude, v);
}

void joystick_update(void)
{
   pos_t p = joystick_read_raw_adc();

   if (calib_state_g.running){
      joystick_update_calibration();
      return;
   }

   pos_g.f.x = pos_g.f.x * (1.0f - FILTER_ALPHA) + p.i.x * FILTER_ALPHA;
   pos_g.f.y = pos_g.f.y * (1.0f - FILTER_ALPHA) + p.i.y * FILTER_ALPHA;

   pos_g.i.x = (int)pos_g.f.x;
   pos_g.i.y = (int)pos_g.f.y;

   // update calibration data
   if (pos_g.i.x < calib_data_g.min.x)
      calib_data_g.min.x = pos_g.i.x;
   
   if (pos_g.i.x > calib_data_g.max.x)
      calib_data_g.max.x = pos_g.i.x;

   if (pos_g.i.y < calib_data_g.min.x)
      calib_data_g.min.y = pos_g.i.y;
   
   if (pos_g.i.y > calib_data_g.max.y)
      calib_data_g.max.y = pos_g.i.y;

   // f.r out i.data
   pos_g.i.x = normalize_axis(pos_g.i.x, calib_data_g.center.x, 
                           calib_data_g.min.x, calib_data_g.max.x);
   pos_g.i.x = deadzone(pos_g.f.x);
   
   pos_g.i.y = normalize_axis(pos_g.i.y, calib_data_g.center.y, 
                           calib_data_g.min.y, calib_data_g.max.y);
   pos_g.i.y = deadzone(pos_g.f.y);

}

pos_t joystick_get_pos(void)
{
   return pos_g;
}

bool joystick_is_calibrating(void)
{
   return calib_state_g.running;
}


esp_err_t joystick_init(void)
{
   esp_err_t ret; 
   adc_oneshot_unit_init_cfg_t cfg = {
      .unit_id = JOY_ADC_UNIT,
   };

   ret = adc_oneshot_new_unit(&cfg, &adc_handle_g);
   ESP_ERROR_CHECK(ret);

   adc_oneshot_chan_cfg_t chn_cfg = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
   };

   ret = adc_oneshot_config_channel(adc_handle_g, JOY_X_CHANNEL, &chn_cfg);
   ESP_ERROR_CHECK(ret);
   ret = adc_oneshot_config_channel(adc_handle_g, JOY_Y_CHANNEL, &chn_cfg);
   ESP_ERROR_CHECK(ret);

   if (calibration_load() != ESP_OK){
      ESP_LOGI("JT", "no calibration data found");
      joystick_start_calibration();
   } else {
      ESP_LOGI("JT", "calibration data found:"
                     " center = (%d, %d)"
                     " range x: %d..%d"
                     " range y: %d..%d",
                     calib_data_g.center.x, 
                     calib_data_g.center.y,
                     calib_data_g.min.x,
                     calib_data_g.max.x,
                     calib_data_g.min.y,
                     calib_data_g.max.y
                     );
   }
   memset(&pos_g, 0, sizeof(pos_g));
   return ret;
}

