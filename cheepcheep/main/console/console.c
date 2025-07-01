#include "console.h"
#include "bsp.h"

#include "esp_console.h"

#define PROMPT_STR "cheep"

esp_console_repl_t *repl = NULL;

status_t console_start(void)
{
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 1020;
    
    esp_console_register_help_command();
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_new_repl_uart(&hw_config, &repl_config, &repl);

    esp_console_start_repl(repl);
    return STATUS_OK;
}

status_t console_register(char *cmd, char *help, char *hint, cli_cb_t cb)
{
    const esp_console_cmd_t _cmd = {
        .command = cmd,
        .help = help,
        .hint = hint,
        .func = cb,
    };  
    esp_console_cmd_register(&_cmd); 
    return STATUS_OK;
}