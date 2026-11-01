#ifndef UDP_H
#define UDP_H

#include "joystick.h"
#include <esp_err.h>

esp_err_t udp_client_init(void);
esp_err_t udp_send_payload(void* data, size_t size);
esp_err_t send_pos(pos_t pos);

#endif 
