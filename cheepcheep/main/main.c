/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp.h"
#include "wiegand.h"
#include "log.h"
#include "config.h"
#include "nvstate.h"
#include "device.h"
#include "tags.h"

wieg_evt_handle_t evt_handle;
device_t *device;

void app_main(void)
{
    status_t status;

    INFO("Getting config");
    const config_t *config = config_get();

    INFO("Setting up gpio");
    status = gpio_init(&config->pins, &config->general);
    if (status != STATUS_OK) { ERROR("gpio init failed: %d"); }

    // Set default pin states
    gpio_out_set(OUTPUT_READER_BUZZER, false);
    gpio_out_set(OUTPUT_READER_LED, false);
    
    INFO("Setting up storage");
    status = fs_init();
    if (status != STATUS_OK) { ERROR("fs init failed: %d"); }

    status = nvstate_init();
    if (status != STATUS_OK) { ERROR("nvstate init failed: %d"); }

    if (config->general.wiegand_enabled)
    {
        INFO("Setting up reader");
        status = wieg_init(
            config->pins.wiegand_zero, 
            config->pins.wiegand_one, 
            config->debug.uid_32bit_mode ? WIEG_34_BIT : WIEG_26_BIT
        );
        if (status != STATUS_OK) { ERROR("wiegand init failed: %d"); }
    }
    else
    {
        ERROR("Configuration Error: Wiegand must be enabled");
        // TODO: Add rdm6300 lib
    }

    INFO("Setting up authorized tag db");
    status = tags_init();
    if (status != STATUS_OK) { ERROR("tags init failed: %d"); }

    switch(config->device_type) {
        case DEVICE_DOOR:
            INFO("Loading door config");
            device = &door;
            break;
        case DEVICE_INTERLOCK:
            INFO("Loading interlock config");
            device = &interlock;
            break;
        case DEVICE_VENDING:
            INFO("Loading vending config");
            device = &vending;
            break;
        default:
            ERROR("Invalid device specified: %d.\nCheck configuration and reflash.");
    }

    INFO("Initializing device");
    status = device->init(config);
    if (status != STATUS_OK) { ERROR("device init failed: %d"); }

    // TODO: the evt handle should be owned by the device, right???
    evt_handle = wieg_evt_handler_reg(WIEG_EVT_NEWCARD, device->swipe_cb, NULL);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}