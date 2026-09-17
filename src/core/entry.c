#include "fribot.h"
#include "blink.h"
#include "imu.h"

void app_main(void) {

   blink_setup();

   imu_handle_t imu_handle;
   imu_init(&imu_handle, imu_default_config());
}
