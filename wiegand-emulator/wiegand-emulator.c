#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "wiegand.h"
#include "bsp.h"
#include "cmd.h"

int main()
{
    // Set up command processor
    cmd_init(UART_ID);

    // Set up wiegand generator
    status_t status = wiegand_init(WIEGAND_D0_PIN, WIEGAND_D1_PIN, 20, 80);

    while (true) 
    {
        // Consume incoming commands
        cmd_task();
        sleep_ms(10);
    }
}