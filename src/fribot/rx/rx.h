#ifndef RX_H
#define RX_H


#include <esp_err.h>
typedef struct _rx_data {
   int16_t *data;
   size_t  size;
   int64_t last_ms;
} rx_data_t;


esp_err_t udp_server_init(void);
void rx_task(void* arg);
rx_data_t rx_get_data(void);

#endif 
