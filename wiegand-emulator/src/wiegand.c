#include "wiegand.h"
#include "status.h"
#include "cmd.h"

#include "pico/stdlib.h"
#include <string.h>
#include <stdlib.h>

typedef struct 
{
    uint32_t bits;
    int ptr;
} wieg_data_t;

typedef struct 
{
    uint p_width;
    uint p_int;
    wieg_data_t data;
    repeating_timer_t timer;
    uint pins[2];
} wieg_ctx_t;

static wieg_ctx_t _ctx;

bool wiegand_handler(int argc, char *argv[]);
bool pulse_int_cb(repeating_timer_t *rt);
int64_t reset_cb(alarm_id_t id, void *user_data);

status_t wiegand_init(int d0, int d1, int p_width, int p_int)
{
    _ctx.pins[0] = d0;
    _ctx.pins[1] = d1;

    gpio_init(d0);
    gpio_set_dir(d0, GPIO_OUT);
    gpio_put(d0, true); // Start HIGH

    gpio_init(d1);
    gpio_set_dir(d1, GPIO_OUT);
    gpio_put(d1, true); // Start HIGH

    wiegand_pulse_set(p_width, p_int);

    return cmd_handler_register(wiegand_handler);
}

void wiegand_pulse_set(int p_width, int p_int)
{
    _ctx.p_width = p_width;
    _ctx.p_int = p_int;
}

status_t wiegand_send(uint8_t facility, uint16_t user_id)
{
    bool leading_parity = 0;  // even parity, first 12 bits
    bool trailing_parity = 1; // odd parity, last 12 bits

    _ctx.data.bits = 0;

    // Pack facility data
    for(int i=0; i<8; i++)
    {
        _ctx.data.bits |= ((facility >> i) & 1) << 8-i;
    }

    // Pack user data
    for(int i=0; i<16; i++)
    {
        _ctx.data.bits |= ((user_id >> i) & 1) << (8+16)-i;
    }

    // Calc leading parity
    for(int i=0; i<12; i++)
    {
        leading_parity ^= ((_ctx.data.bits >> (i+1)) & 1);
    }

    // Calc trailing parity
    for(int i=0; i<12; i++)
    {
        trailing_parity ^= ((_ctx.data.bits >> (i+12+1)) & 1);
    }
    
    // Pack parity bits
    _ctx.data.bits |= leading_parity;
    _ctx.data.bits |= trailing_parity << 25;

    // Reset pointer
    _ctx.data.ptr = 0;

    // Set up repeating timer according to pulse interval
    add_repeating_timer_us(_ctx.p_int, pulse_int_cb, NULL, &_ctx.timer);
    return STATUS_OK;
}

bool pulse_int_cb(repeating_timer_t *rt) 
{
    bool bit = (_ctx.data.bits >> _ctx.data.ptr) & 1;

    // Schedule the timer to reset the bit according to pulse width
    add_alarm_in_us(_ctx.p_width, reset_cb, (void *)_ctx.pins[bit], true);

    // Set data pin LOW
    gpio_put(_ctx.pins[bit], false);
    
    // Reschedule until 26 bits have been sent
    return _ctx.data.ptr++ < 26;
}

int64_t reset_cb(alarm_id_t id, void *user_data)
{
    // Set data pin HIGH again
    uint pin = (int) user_data;
    gpio_put(pin, true);

    // pulse_int_cb schedules this, don't need to return to resched
    return 0;
}


bool wiegand_handler(int argc, char *argv[])
{
    if (strcmp(argv[0], "pulse") == 0 || strcmp(argv[0], "PULSE") == 0)
    {
        // PULSE width interval - sets the pulse characteristics
        if (argc == 3)
        {
            uint8_t w = atoi(argv[1]);
            uint8_t i = atoi(argv[2]);
            wiegand_pulse_set(w, i);
            return true;
        }
    }
    else if (strcmp(argv[0], "send") == 0 || strcmp(argv[0], "SEND") == 0)
    {
        // SEND facility user_id - emulates wiegand signal
        if (argc == 3)
        {
            uint8_t f = atoi(argv[1]);
            uint16_t u = atoi(argv[2]);
            wiegand_send(f, u);
            return true;
        }
    }

    return false;
}
