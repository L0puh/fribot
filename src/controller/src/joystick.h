#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "config.h"
#include <esp_err.h>
#include <stdint.h>

#define CALIB_SAMPLES      1000
#define CENTER_ADAPT_ALPHA 0.001f
#define DEADZONE           0.05f
#define FILTER_ALPHA       0.05f 
#define MIN_VALID_RANGE    100

typedef struct {
   struct {
      int x, y;
   } i;
   struct {
      float x, y;
   } f;
} pos_t;

typedef struct {
   struct {
      int x, y;
   } center;
   struct {
      int x, y;
   } min;
   struct {
      int x, y;
   } max;

   uint32_t version;
} calib_t;

typedef struct {
   bool running;
   int count;
   int sum_x, sum_y;
} calib_state_t;

//NVS
esp_err_t calibration_load(void);
esp_err_t calibration_save(void);

//ADC
pos_t joystick_read_raw_adc(void);

//calibration
void joystick_start_calibration(void);
void joystick_update_calibration(void);

//helpers
float normalize_axis(int raw, int center, int min, int max);
float deadzone(float v);

//public
void   joystick_update(void);
pos_t  joystick_get_pos(void);
bool   joystick_is_calibrating(void);
esp_err_t joystick_init(void);


#endif 
