#ifndef SONAR_H
#define SONAR_H

#include "config.h"
#include <driver/gpio.h>
#include <driver/rmt_rx.h>
#include <stdint.h>

#define RMT_RESOLUTION_HZ 1000000   // 1 tick = 1 us
#define TRIGGER_US        10
#define MAX_DISTANCE_CM   400 

typedef struct {
   rmt_channel_handle_t rmt_channel_g;
   rmt_symbol_word_t rmt_symbol_g[8]; 
   volatile bool measurement_done;
   volatile uint32_t echo_time_us;
} sonar_t;

esp_err_t sonar_init(void);
float sonar_measure_cm(void);

#endif 
