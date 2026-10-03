#include "motors.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_types.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "hal/mcpwm_types.h"
#include <stdint.h>


static int clamp_speed(int speed) {
   if (speed > MOTORS_MAXIMUM_SPEED)  speed = MOTORS_MAXIMUM_SPEED;
   if (speed < -MOTORS_MAXIMUM_SPEED) speed = -MOTORS_MAXIMUM_SPEED;
   return speed;
}

static esp_err_t motor_set_pwm_pin(const mcpwm_gen_handle_t gen,
                                  const mcpwm_cmpr_handle_t cmpr, int speed)
{
   esp_err_t ret;
   uint32_t ticks = (uint32_t)speed * MOTORS_PWM_PERIOD / 255;

   ret = mcpwm_comparator_set_compare_value(cmpr, ticks);
   ESP_ERROR_CHECK(ret);
   
   ret = mcpwm_generator_set_force_level(gen, -1, true);

   return ret;
}

static esp_err_t motor_set_low(const mcpwm_gen_handle_t gen)
{
   esp_err_t ret;
   ret = mcpwm_generator_set_force_level(gen, 0, true);
   return ret;
}

static esp_err_t motor_timer_set_actions(motor_t* motor)
{
   esp_err_t ret;
   mcpwm_gen_timer_event_action_t action1;
   mcpwm_gen_compare_event_action_t action2, action3;

   // high when timer restarts, low when compare value
   action1 = MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, 
                                          MCPWM_TIMER_EVENT_EMPTY, 
                                          MCPWM_GEN_ACTION_HIGH);

   action2 = MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
                                            motor->cm1,
                                            MCPWM_GEN_ACTION_LOW);
   
   action3 = MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
                                            motor->cm2,
                                            MCPWM_GEN_ACTION_LOW);

   ret = mcpwm_generator_set_action_on_timer_event(motor->gen1, action1);
   ESP_ERROR_CHECK(ret);
   ret = mcpwm_generator_set_action_on_compare_event(motor->gen1, action2);
   ESP_ERROR_CHECK(ret);

   ret = mcpwm_generator_set_action_on_timer_event(motor->gen2, action1);
   ESP_ERROR_CHECK(ret);
   ret = mcpwm_generator_set_action_on_compare_event(motor->gen2, action3);
   return ret;
}

esp_err_t motor_set_speed(motor_t *motor, int speed)
{
   esp_err_t ret;

   speed = clamp_speed(speed);

   if (speed > 0) {
      ret = motor_set_pwm_pin(motor->gen1, motor->cm1, speed);
      ESP_ERROR_CHECK(ret);
      ret = motor_set_low(motor->gen2);
   } else if (speed < 0){
      ret = motor_set_low(motor->gen1);
      ESP_ERROR_CHECK(ret);
      ret = motor_set_pwm_pin(motor->gen2, motor->cm2, -speed);
   } else {
      ret = motor_set_low(motor->gen1);
      ESP_ERROR_CHECK(ret);
      ret = motor_set_low(motor->gen2);
   }

   return ret;
}

esp_err_t motor_init(motor_t *motor, mcpwm_timer_handle_t timer, int pin1, int pin2)
{
   esp_err_t ret;

   mcpwm_oper_handle_t operator;
   mcpwm_operator_config_t oper_cfg = {.group_id=0 }; 
   mcpwm_comparator_config_t cmp_cfg = { .flags.update_cmp_on_tez = true };
   mcpwm_generator_config_t gen1_cfg = { .gen_gpio_num = pin1 };
   mcpwm_generator_config_t gen2_cfg = { .gen_gpio_num = pin2 };


   ret = mcpwm_new_operator(&oper_cfg, &operator);
   ESP_ERROR_CHECK(ret);
   ret = mcpwm_operator_connect_timer(operator, timer);
   ESP_ERROR_CHECK(ret);

   ret = mcpwm_new_comparator(operator, &cmp_cfg, &motor->cm1);
   ESP_ERROR_CHECK(ret);
   ret = mcpwm_new_comparator(operator, &cmp_cfg, &motor->cm2);
   ESP_ERROR_CHECK(ret);

   ret = mcpwm_new_generator(operator, &gen1_cfg, &motor->gen1);
   ESP_ERROR_CHECK(ret);
   ret = mcpwm_new_generator(operator, &gen2_cfg, &motor->gen2);
   ESP_ERROR_CHECK(ret);

   ret = motor_timer_set_actions(motor);
   ESP_ERROR_CHECK(ret);

   ESP_ERROR_CHECK(mcpwm_generator_set_force_level(motor->gen1, 0, true));
   ESP_ERROR_CHECK(mcpwm_generator_set_force_level(motor->gen2, 0, true));

   return ret;
}

esp_err_t motor_forward(motor_t* motor, int speed)
{
   if (motor == NULL) return ESP_ERR_INVALID_ARG;
   if (speed < 0) speed = 0;
   
   return motor_set_speed(motor, speed);
}

esp_err_t motor_backward(motor_t* motor, int speed)
{
   if (motor == NULL) return ESP_ERR_INVALID_ARG;
   if (speed < 0) speed = 0;
   
   return motor_set_speed(motor, -speed);
}

esp_err_t motor_stop(motor_t* motor)
{
   return motor_set_speed(motor, 0);
}
