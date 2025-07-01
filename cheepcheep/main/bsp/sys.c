#include "sys.h"

void
sys_enter_boot (void)
{
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    esp_restart();
}

void sys_restart(void)
{
    esp_restart();
}

