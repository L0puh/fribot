#ifndef CONTEXT_H
#define CONTEXT_H


#include "imu.h"
#include "motors.h"

typedef struct _context {
   imu_handle_t imu_handle;
   imu_raw_data_t raw_data;
   motor_t motors[2];
} context_t;

extern context_t ctx_g;
context_t ctx_g;

#endif 
