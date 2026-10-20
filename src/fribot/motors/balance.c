#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_types.h"
#include "esp_err.h"
#include "motors.h"
#include "fribot.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

esp_err_t setup_motors(motor_t* motor_a, motor_t *motor_b)
{
   esp_err_t ret;

   mcpwm_timer_handle_t timer;
   mcpwm_timer_config_t timer_cfg = {  
      .group_id = 0,
      .clk_src = MOTORS_TIMER_CLOCK_SOURCE,
      .resolution_hz = MOTORS_TIMER_RESOLUTION,
      .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
      .period_ticks = MOTORS_PWM_PERIOD,
   };

   ret = mcpwm_new_timer(&timer_cfg, &timer);
   ESP_ERROR_CHECK(ret);

   motor_init(motor_a, timer, MOTOR_A_IN1, MOTOR_A_IN2);
   motor_init(motor_b, timer, MOTOR_B_IN1, MOTOR_B_IN2);
  
   ret = mcpwm_timer_enable(timer);
   ESP_ERROR_CHECK(ret);
   ret = mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
   ESP_ERROR_CHECK(ret);

   return ret;
}
