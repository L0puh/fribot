#ifndef MENU_H
#define MENU_H


#include <esp_err.h>

#define DEBOUNCH_TIME_MS 30

typedef struct {

   struct {
      bool is_pressed;
      bool last_reading;
      int64_t last_change_ms;

      bool press_event;
      bool release_event;
   } button;
} menu_t;



esp_err_t menu_update();
esp_err_t menu_init(void);
menu_t menu_get_state(void);

#endif 
