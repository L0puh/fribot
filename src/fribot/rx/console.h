#ifndef CONSOLE_H
#define CONSOLE_H

#include <esp_err.h>

esp_err_t setup_console(int (*func)(int argc, char**));
#endif 
