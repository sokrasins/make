#include "sys.h"

#include "esp_system.h"
#include "soc/rtc_cntl_reg.h"

void sys_enter_boot (void)
{
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    esp_restart();
}

void sys_restart(void)
{
    esp_restart();
}

