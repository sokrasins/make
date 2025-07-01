#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "status.h"

typedef int (*cli_cb_t)(int argc, char **argv);

status_t console_start(void);

status_t console_register(char *cmd, char *help, char *hint, cli_cb_t cb);

#endif /*CONSOLE_H_*/