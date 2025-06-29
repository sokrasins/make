#include "uptime.h"

#include "esp_timer.h"

int64_t uptime(void)
{
    return esp_timer_get_time() / 1000;
}