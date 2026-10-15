#ifndef CMD_H 
#define CMD_H 


#include <esp_err.h>
#include <stdint.h>

//TODO: decide on the schema  

typedef enum _cmd {
   CMD_MOTOR_A   = 1,
   CMD_MOTOR_B   = 2 ,
   CMD_HEARTBEAT = 3,
   CMD_DEBUG     = 4,

   CMD_SIZE_ENUM,
   INVALID_CMD_ERROR
} cmd_type_e;


const char* cmd_names[CMD_SIZE_ENUM] = {
   [CMD_MOTOR_A]   = "CMD_MOTOR_A",
   [CMD_MOTOR_B]   = "CMD_MOTOR_B",
   [CMD_HEARTBEAT] = "CMD_HEARTBEAT",
   [CMD_DEBUG]     = "CMD_DEBUG",
};

typedef struct _cmd_t {
   uint8_t  id;
   uint8_t  type; 
   uint32_t payload;
   uint16_t reserved;
} cmd_t;


// public:
esp_err_t parse_cmd(const uint64_t raw, cmd_t* out_cmd);
const char* get_cmd_type_name(const uint8_t type);

// private:
cmd_type_e parse_cmd_type(const uint64_t raw);
uint32_t parse_cmd_payload(const uint64_t raw);
uint16_t parse_cmd_reserved(const uint64_t raw);
esp_err_t parse_cmd(const uint64_t raw, cmd_t* out_cmd);


#endif
