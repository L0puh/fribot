#include "cmd.h"

const char* get_cmd_type_name(const uint8_t type)
{
   if (type > CMD_SIZE_ENUM) return NULL;

   return cmd_names[type];
}

cmd_type_e parse_cmd_type(const uint64_t raw)
{
   uint8_t cmd_type;

   cmd_type = raw >> 8;
   if (get_cmd_type_name(cmd_type) == NULL){
      return INVALID_CMD_ERROR;
   }

   return (cmd_type_e) cmd_type;
}

uint32_t parse_cmd_payload(const uint64_t raw)
{
   uint32_t data;
   data = (uint32_t)(raw >> 16);
   return data;
}

uint16_t parse_cmd_reserved(const uint64_t raw)
{
   uint16_t reserved;
   reserved = (uint16_t)(raw >> 32);
   //TODO: parse it out 

   return reserved;
}


// assume raw is LSB package 
esp_err_t parse_cmd(const uint64_t raw, cmd_t* out_cmd)
{
   if (raw ==  0 || out_cmd == NULL)
      return ESP_ERR_INVALID_ARG;

   out_cmd->id = raw & 0xFF;
   out_cmd->type = parse_cmd_type(raw);
   out_cmd->payload = parse_cmd_payload(raw);
   out_cmd->reserved = parse_cmd_reserved(raw);


   if (out_cmd->type == INVALID_CMD_ERROR) {
      return ESP_ERR_INVALID_STATE;
   }

   return ESP_OK;
}
