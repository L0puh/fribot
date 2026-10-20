#ifndef WIFI_H
#define WIFI_H

#include <esp_err.h>
#include <esp_event.h>

esp_err_t wifi_init_softap(void);
bool wifi_is_station_connected(void);

#endif 
