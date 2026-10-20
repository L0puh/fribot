#include "wifi.h"
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <lwip/err.h>
#include <lwip/sys.h>

#include "config.h"

static volatile bool g_station_connected = false;

bool wifi_is_station_connected(void)
{
   return g_station_connected;
}

static void wifi_event_handler(void* arg, esp_event_base_t even_base,
                                       int32_t event_id, void* data)
{
   switch(event_id)
   {
      case WIFI_EVENT_AP_STACONNECTED:
         {
            wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) data;
            ESP_LOGI("wifi", "connection AID=%d from MAC "MACSTR"",
                                       event->aid, MAC2STR(event->mac));
            g_station_connected = true;
            break;
         }
      case WIFI_EVENT_AP_STADISCONNECTED:
         {
            wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) data;
            ESP_LOGI("wifi", "disconnected AID=%d MAC "MACSTR" (reason=%d)", event->aid, 
                  MAC2STR(event->mac));
            g_station_connected = false;
            break;
         }

      default: break;
      
   }
}

esp_err_t wifi_init_softap(void)
{

   esp_err_t ret; 

   ret = esp_netif_init();
   ESP_ERROR_CHECK(ret);

   ret = esp_event_loop_create_default();
   ESP_ERROR_CHECK(ret);

   esp_netif_create_default_wifi_ap();

   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

   ret = esp_wifi_init(&cfg);
   ESP_ERROR_CHECK(ret);

   ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                          &wifi_event_handler, NULL, NULL);
   ESP_ERROR_CHECK(ret);

   wifi_config_t wifi_config = {
      .ap = {
         .ssid = FRIBOT_WIFI_SSID,
         .ssid_len = strlen(FRIBOT_WIFI_SSID),
         .channel = FRIBOT_WIFI_CHANNEL, 
         .password = FRIBOT_WIFI_PSWD,
         .max_connection = FRIBOT_MAX_STA_CONN,
         .authmode = WIFI_AUTH_WPA2_PSK,
         .pmf_cfg = { .required = true },
         .bss_max_idle_cfg = {
             .period = WIFI_AP_DEFAULT_MAX_IDLE_PERIOD,
             .protected_keep_alive = 1,
         },
         .gtk_rekey_interval = 0,
      },
   };

   if (strlen(FRIBOT_WIFI_PSWD) == 0 && wifi_config.ap.authmode != WIFI_AUTH_OWE) {
      wifi_config.ap.authmode = WIFI_AUTH_OPEN;
   }

   ret = esp_wifi_set_mode(WIFI_MODE_AP);
   ESP_ERROR_CHECK(ret);
   ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
   ESP_ERROR_CHECK(ret);
   ret = esp_wifi_start();
   ESP_ERROR_CHECK(ret);

   ESP_LOGI("wifi", "wifi init finished, SSID: %s PSWD: %s CHAN: %d", FRIBOT_WIFI_SSID, 
            FRIBOT_WIFI_PSWD, FRIBOT_WIFI_CHANNEL);

   return ret;
}


