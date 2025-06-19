#include "cmd.h"

#include "bsp.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_HANDLERS_MAX 20U
#define CMD_BUF_SIZE 1024U

int cmd_send(const char *fmt,...);
void on_uart_rx(void);
void process_data(uint8_t *data, size_t len);
bool parse_cmd(int argc, char *argv[]);
bool cmd_handler(int argc, char *argv[]);

typedef struct {
    uart_inst_t *uart;
    uint8_t in_buf[CMD_BUF_SIZE];
    size_t in_len;
    uint8_t out_buf[CMD_BUF_SIZE];
    cmd_handler_t handlers[CMD_HANDLERS_MAX];
    bool process_data;
    bool echo;
} cmd_ctx_t;

static cmd_ctx_t _ctx = {
    .process_data = false,
    .echo = false,
};

void cmd_init(uart_inst_t *uart)
{
    _ctx.uart = uart;

    // Set up our UART
    uart_init(_ctx.uart, BAUD_RATE);
    uart_getc(_ctx.uart);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    int UART_IRQ = _ctx.uart == uart0 ? UART0_IRQ : UART1_IRQ;

    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    uart_set_irqs_enabled(_ctx.uart, true, false);

    for (int i=0; i<CMD_HANDLERS_MAX; i++)
    {
        _ctx.handlers[i] = NULL;
    }

    cmd_handler_register(cmd_handler);
}

status_t cmd_handler_register(cmd_handler_t handler)
{
    for (int i=0; i<CMD_HANDLERS_MAX; i++)
    {
        if (_ctx.handlers[i] == NULL)
        {
            _ctx.handlers[i] = handler;
            return STATUS_OK;
        }
    }

    return -STATUS_NOMEM;
}

status_t cmd_handler_deregister(cmd_handler_t handler)
{
    for (int i=0; i<CMD_HANDLERS_MAX; i++)
    {
        if (_ctx.handlers[i] == handler)
        {
            _ctx.handlers[i] = NULL;
            return STATUS_OK;
        }
    }

    return -STATUS_NXHANDLE;   
}

int cmd_send(const char *fmt,...)
{
    int n;
    va_list args;
  
    va_start (args, fmt);
    n = vsprintf(_ctx.out_buf, fmt, args);
    uart_puts(UART_ID, _ctx.out_buf);
    va_end(args);

    return n;  
}

void cmd_task(void)
{
    if (_ctx.process_data)
    {
        process_data(_ctx.in_buf, _ctx.in_len);
        memset(_ctx.in_buf, 0, _ctx.in_len);
        _ctx.in_len = 0;
        _ctx.process_data = false;
    }
}

bool cmd_handler(int argc, char *argv[])
{

    if (strcmp(argv[0], "echo") == 0 || strcmp(argv[0], "ECHO") == 0)
    {
        // ECHO status - Enables/disables command echo on the interface
        if (argc == 2)
        {
            uint8_t e = atoi(argv[1]);
            if (e) _ctx.echo = true;
            else _ctx.echo = false;
            return true;
        }
    }

    return false;
}

void on_uart_rx(void) {
    while (uart_is_readable(_ctx.uart)) 
    {
        char c = uart_getc(_ctx.uart);
        if (_ctx.echo) cmd_send("%c", c);

        if (c == '\r')
        {       
            if (_ctx.echo) cmd_send("\n");

            // Set a flag for processing outside the ISR
            _ctx.process_data = true;
        }
        else if(c == '\b')
        {
            // TODO: Doesn't work
            if (_ctx.in_len > 0)
            {
                _ctx.in_len--;
                _ctx.in_buf[_ctx.in_len] = '\0'; 
            }
        }
        else 
        {
            _ctx.in_buf[_ctx.in_len++] = c;
        }
    
        if(_ctx.in_len >= CMD_BUF_SIZE) 
        {
            memset(_ctx.in_buf, 0, CMD_BUF_SIZE);
            break;
        }
    }
}

void process_data(uint8_t *data, size_t len)
{
    int argc = 1;
    char *argv[20];

    argv[0] = &data[0];

    for (int i=0; i<len; i++)
    {
        if (data[i] == ' ')
        {   data[i] = '\0';
            argv[argc++] = &data[i+1];
        }
    }
    argv[argc] = '\0';

    bool handled = false;
    for (int i=0; i<CMD_HANDLERS_MAX; i++)
    {
        cmd_handler_t handler = _ctx.handlers[i];
        if (handler != NULL)
        {
            handled = handler(argc, argv);
            if (handled)
            {
                break;
            }
        }
    }

    if (handled)
    {
        cmd_send("OK\r\n");
    }
    else
    {
        cmd_send("KO\r\n");
    }
}