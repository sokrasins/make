#ifndef LOGGER_H_
#define LOGGER_H_

#include "status.h"
#include "bsp.h"
#include <stdbool.h>

/** 
 * @brief Command handler type
 * @param argc Number of arguments in argv
 * @param argv Input params
 * @return true if the command has been handled by this handler. false otherwise.
 **/
typedef bool (*cmd_handler_t)(int arc, char *argv[]);

/** 
 * @brief Initialize the command handler. This must be done before any handlers 
 * are registered.asm
 * @param uart UART port for command interface
 **/
void cmd_init(uart_inst_t *uart);

/** 
 * @brief Register command handlers. Registered handlers will be called until 
 * one handler processes the command (returns true).
 * @param handler to register
 * @return STATUS_NOMEM if more hanlders cannot be registered
 *         STATUS_OK otherwise
 **/
status_t cmd_handler_register(cmd_handler_t handler);

/** 
 * @brief Deregister command handlers
 * @param handler to deregister
 * @return STATUS_NXHANDLE if handler wasn't registered
 *         STATUS_OK otherwise
 **/
status_t cmd_handler_deregister(cmd_handler_t handler);

/** 
 * @brief Task for processing incoming commands. This task must be called in the 
 * main loop (as there's no RTOS here). This saves command processing from happening 
 * in an ISR.
 **/
void cmd_task(void);

#endif /*LOGGER_H_*/