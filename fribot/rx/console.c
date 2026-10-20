#include <esp_err.h>
#include <esp_console.h>

esp_err_t setup_console(int (*func)(int argc, char**))
{
   esp_err_t ret; 
   esp_console_repl_t *repl = NULL;
   esp_console_repl_config_t cfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
   esp_console_dev_uart_config_t uart_cfg = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();

   cfg.prompt = "dbg> ";
   ret = esp_console_new_repl_uart(&uart_cfg, &cfg, &repl);
   ESP_ERROR_CHECK(ret);

   const esp_console_cmd_t cmd = {
      .command = "toggle",
      .help = "toggle <motor_a|motor_b|imu>",
      .hint = NULL,
      .func = func 
   };

   ret = esp_console_cmd_register(&cmd);
   ESP_ERROR_CHECK(ret);

   return esp_console_start_repl(repl);
}
