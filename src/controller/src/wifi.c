#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_err.h>

#include <lwip/err.h>
#include <lwip/sys.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include "config.h"
#include "wifi.h"

static EventGroupHandle_t s_wifi_event_group;

esp_err_t wifi_wait_connection(void)
{
   // wait until connected / aborted
   EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
       pdFALSE, pdFALSE, portMAX_DELAY);

   if (bits & WIFI_CONNECTED_BIT){
      ESP_LOGI("wifi", "connected to bot (SSID: %s PSWD: %s)", FRIBOT_WIFI_SSID, FRIBOT_WIFI_PSWD);
      return ESP_OK;
   } 

   if (bits & WIFI_FAIL_BIT){
      ESP_LOGI("wifi", "fail to connect to bot (SSID: %s PSWD: %s)", FRIBOT_WIFI_SSID, FRIBOT_WIFI_PSWD);
      return ESP_FAIL;
   } 

   ESP_LOGI("wifi", "unexpected event");
   return ESP_ERR_NOT_FOUND;
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
   static int s_retry_num = 0;
   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
      return;
   }

   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if (s_retry_num < WIFI_MAX_RETRY_NUM) {
         esp_wifi_connect();
         s_retry_num++;
         ESP_LOGI("wifi", "retry to connect to the AP");
      } else {
         xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      }
      ESP_LOGI("wifi","connect to the AP fail");
      return;
   } 

   if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
      ESP_LOGI("wifi", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
      s_retry_num = 0;
      xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
   }
}

esp_err_t wifi_station_init(void) 
{
   esp_err_t ret;
   s_wifi_event_group = xEventGroupCreate();

   ret = esp_netif_init();
   ESP_ERROR_CHECK(ret);

   ret = esp_event_loop_create_default();
   ESP_ERROR_CHECK(ret);

   esp_netif_create_default_wifi_sta();

   esp_event_handler_instance_t inst_any_id, inst_got_ip;

   ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
         &event_handler, NULL, &inst_any_id);

   ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
         &event_handler, NULL, &inst_got_ip);


   wifi_config_t cfg = {
      .sta = {
         .ssid = FRIBOT_WIFI_SSID,
         .password = FRIBOT_WIFI_PSWD,
         .threshold.authmode = WIFI_AUTH_WPA2_PSK,
         .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
         .disable_wpa3_compatible_mode = 1,
      },
   };

   ret = esp_wifi_set_mode(WIFI_MODE_STA);
   ESP_ERROR_CHECK(ret);
   ret = esp_wifi_set_config(WIFI_IF_STA, &cfg);
   ESP_ERROR_CHECK(ret);

   ESP_LOGI("wifi", "wifi init is finished");
   return wifi_wait_connection();
}


