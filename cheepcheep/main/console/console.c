#include "console.h"
#include "bsp.h"

#include "esp_console.h"

#define PROMPT_STR "cheep"

static void register_restart(void);
static int _restart(int argc, char **argv);

esp_console_repl_t *repl = NULL;

status_t console_init(void)
{
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 1020;
    
    esp_console_register_help_command();

    register_restart();
    
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_new_repl_uart(&hw_config, &repl_config, &repl);

    esp_console_start_repl(repl);

    return STATUS_OK;
}

static void register_restart(void)
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &_restart,
    };
    esp_console_cmd_register(&cmd);
}

static int _restart(int argc, char **argv)
{
    printf("Restarting...");
    sys_restart();

    return 0;
}