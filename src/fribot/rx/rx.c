#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <esp_timer.h>

#include <esp_log.h>
#include <esp_err.h>

#include "config.h"
#include "rx.h"

static int socket_g = -1;
static rx_data_t rx_data_g;

esp_err_t udp_server_init(void)
{
   int ret;
   socket_g = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

   if (socket_g < 0)
   {
      ESP_LOGE("rx", "failed init socket: %d", errno);
      return ESP_FAIL;
   }

   struct sockaddr_in server_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(FRIBOT_PORT),
      .sin_addr.s_addr = htonl(INADDR_ANY),
   };

   ret = bind(socket_g, (struct sockaddr*)&server_addr, sizeof(server_addr));
   if (ret < 0)
   {
      ESP_LOGE("rx", "bind failed: %d", errno);
      close(socket_g);
      socket_g = -1;
      return ESP_FAIL;
   }

   ESP_LOGI("rx", "UDP server listening on %s:%d", FRIBOT_IP, FRIBOT_PORT);
   return ESP_OK;
}

rx_data_t rx_get_data(void)
{
   return rx_data_g;
}

void rx_task(void* arg)
{
   int16_t payload[2];
   while (1)
   {
      int len;
      struct sockaddr_in src_addr;
      socklen_t src_addr_len = sizeof(src_addr);

      len = recvfrom(socket_g, payload, sizeof(payload), 0, 
            (struct sockaddr*)&src_addr, &src_addr_len);

      if (len < 0){
         ESP_LOGE("rx", "recvfrom failed: %d", errno);
      }

      if (len != sizeof(payload))
      {
         ESP_LOGW("rx", "invalid packet size: %d (expected %zu)", 
                                          len, sizeof(payload));
         continue;
      }

      ESP_LOGI("rx", "received position: %d %d", payload[0], payload[1]);
      //FIXME: it needs refactoring!!!
      rx_data_g.data = payload;
      rx_data_g.size = len;
      rx_data_g.last_ms = esp_timer_get_time() / 1000;
   }
   close(socket_g);
   socket_g = -1;
   vTaskDelete(NULL);
}


