
// socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <esp_log.h>

#include "config.h"
#include "esp_err.h"
#include "joystick.h"
#include "udp.h"

static struct sockaddr_in dest_addr_g;
static int socket_g = -1;

esp_err_t udp_client_init(void)
{
   if (socket_g < 0)
   {
      // UPD for now, maybe TCP later?? (TODO)
      socket_g = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
      if (socket_g < 0)
      {
         ESP_LOGE("udp", "failed to create UDP socket");
         return ESP_FAIL;
      }

      dest_addr_g.sin_family = AF_INET;
      dest_addr_g.sin_port = htons(FRIBOT_PORT);
      dest_addr_g.sin_addr.s_addr = inet_addr(FRIBOT_IP);
   }

   return ESP_OK;
}

esp_err_t udp_send_payload(void* data, size_t size)
{
   int ret;

   if (size == 0 || data == NULL) {
      return ESP_ERR_INVALID_SIZE;
   }

   ESP_LOGI("udp", "sending to %s:%d", FRIBOT_IP, FRIBOT_PORT);
   ret = sendto(socket_g, data, size, 0,
         (struct sockaddr*)&dest_addr_g, sizeof(dest_addr_g));

   if (ret < 0){
      ESP_LOGE("udp", "Error sending position: %d", errno);
      return ESP_FAIL;
   }

   if ((size_t)ret != size) {
      ESP_LOGE("udp", "only sent %d/%zu bytes", ret, size);
      return ESP_ERR_NOT_FINISHED;
   }

   return ESP_OK;
}


esp_err_t send_pos(pos_t pos)
{
   int16_t payload[2] = { pos.i.x, pos.i.y };
   esp_err_t ret = udp_send_payload(payload, sizeof(payload));
   if (ret != ESP_OK)
   {
      ESP_LOGE("udp", "failed to send position");
      return ret;
   }
   ESP_LOGI("udp", "sent position - OK");
   return ret;
}
