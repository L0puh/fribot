#ifndef WIFI_H
#define WIFI_H

#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_err.h>

#include <lwip/err.h>
#include <lwip/sys.h>

#define WIFI_CONNECTED_BIT BIT0 
#define WIFI_FAIL_BIT      BIT1 

#define WIFI_MAX_RETRY_NUM 10

esp_err_t wifi_station_init(void);
esp_err_t wifi_wait_connection(void);

#endif 
