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
 

#define BLINK_GPIO GPIO_NUM_2
#define BLINK_PERIOD_MS 500

void blink_setup(void);
void blink_times(size_t count);


#endif 
