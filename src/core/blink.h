/************************
 * 
 * @type: core 
 * @about: 
 * LED blink for debugging
 * used to indicate status codes
 *
 *************************/

#ifndef BLINK_H
#define BLINK_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BLINK_DEBUG GPIO_NUM_2
#define BLINK_PERIOD_MS 700

esp_err_t blink_setup(const uint8_t pin);
void blink_times(const uint8_t pin, const uint32_t count);


#endif 
