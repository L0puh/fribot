#ifndef MOTORS_H
#define MOTORS_H

/*
TC1508A driver with two DC motors 
*/

#include "driver/mcpwm_types.h"
#include "soc/clk_tree_defs.h"
#include <esp_err.h>
#include <driver/mcpwm_prelude.h>

#define MOTORS_TIMER_CLOCK_SOURCE (MCPWM_TIMER_CLK_SRC_DEFAULT)
#define MOTORS_TIMER_RESOLUTION   (1000000) // 1MHz 
#define MOTORS_PWM_PERIOD         (1000)    // 1kHz PWM 
#define MOTORS_MAXIMUM_SPEED      (250)     // 0...255

typedef struct _motor_t {
   mcpwm_gen_handle_t gen1;
   mcpwm_gen_handle_t gen2;
   mcpwm_cmpr_handle_t cm1;
   mcpwm_cmpr_handle_t cm2;

} motor_t;


// public: 
esp_err_t setup_motors(motor_t* motor_a, motor_t *motor_b);
esp_err_t motor_stop(motor_t* motor);
esp_err_t motor_backward(motor_t* motor, int speed);
esp_err_t motor_forward(motor_t* motor, int speed);


// private:
esp_err_t motor_init(motor_t *motor, mcpwm_timer_handle_t timer, int pin1, int pin2);
esp_err_t motor_set_speed(motor_t *motor, int speed);


#endif 
