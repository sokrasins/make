#include "signal.h"
#include "gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SIGNAL_OK_FLAG      (1<<0)
#define SIGNAL_ALERT_FLAG   (1<<1)

void signal_task(void *params);

TaskHandle_t signal_task_handle = NULL;

status_t signal_init(void)
{
    xTaskCreate(signal_task, "Signal_Task", 2048, NULL, 2, &signal_task_handle);
    return STATUS_OK;
}

void signal_alert(void)
{
    xTaskNotify(signal_task_handle, SIGNAL_ALERT_FLAG, eSetBits);
}

void signal_ok(void)
{
    xTaskNotify(signal_task_handle, SIGNAL_OK_FLAG, eSetBits);
}

void signal_task(void *params)
{
    while(1)
    {
        uint32_t flags = 0;
        if(xTaskNotifyWait(0x00, 0xFFFFFFFF, &flags, portMAX_DELAY) == pdPASS)
        {
            if(flags & SIGNAL_ALERT_FLAG)
            {
                gpio_out_set(OUTPUT_READER_BUZZER, true);
                gpio_out_set(OUTPUT_READER_LED, true);
                vTaskDelay(pdMS_TO_TICKS(1000));

                gpio_out_set(OUTPUT_READER_BUZZER, false);
                gpio_out_set(OUTPUT_READER_LED, false);
            }
            if(flags & SIGNAL_OK_FLAG)
            {
                gpio_out_set(OUTPUT_READER_BUZZER, true);
                gpio_out_set(OUTPUT_READER_LED, true);
                vTaskDelay(pdMS_TO_TICKS(300));

                gpio_out_set(OUTPUT_READER_BUZZER, false);
                gpio_out_set(OUTPUT_READER_LED, false);
                vTaskDelay(pdMS_TO_TICKS(300));

                gpio_out_set(OUTPUT_READER_BUZZER, true);
                gpio_out_set(OUTPUT_READER_LED, true);
                vTaskDelay(pdMS_TO_TICKS(300));

                gpio_out_set(OUTPUT_READER_BUZZER, false);
                gpio_out_set(OUTPUT_READER_LED, false);
            }
        }
    }
}
