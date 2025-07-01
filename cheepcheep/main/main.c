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
#include "client.h"
#include "ota_dfu.h"
#include "console.h"

#include "esp_app_desc.h"

device_t *device;
const config_t *config;

static status_t server_cmd_handler(msg_t *msg);

static int _reboot(int argc, char **argv);

void app_main(void)
{
    status_t status;

    const esp_app_desc_t *desc = esp_app_get_description();

    INFO("");
    INFO("*********************************");
    INFO("********** CHEEP CHEEP **********");
    INFO("**********   v%s    **********", desc->version);
    INFO("*********************************");
    INFO("Built: %s, %s", desc->date, desc->time);
    INFO("");

    // If the application is new, this will mark it as runnable. Otherwise, the 
    // application may roll back to a previous version
    ota_mark_application(true);

    console_register("restart", "reboot the device", NULL, _reboot);

    status = nvstate_init();
    if (status != STATUS_OK) { ERROR("nvstate_init failed: %d"); }

    INFO("Getting config");
    config = config_get();
    if (config == NULL) { ERROR("Couldn't get device config"); }

    INFO("Starting console");
    console_start();

    INFO("Setting up gpio");
    status = gpio_init(&config->pins, &config->general);
    if (status != STATUS_OK) { ERROR("gpio_init failed: %d"); }

    INFO("Setting up storage");
    status = fs_init();
    if (status != STATUS_OK) { ERROR("fs_init failed: %d"); }

    if (config->general.wiegand_enabled)
    {
        INFO("Setting up reader");
        status = wieg_init(
            config->pins.wiegand_zero, 
            config->pins.wiegand_one, 
            config->debug.uid_32bit_mode ? WIEG_34_BIT : WIEG_26_BIT
        );
        if (status != STATUS_OK) { ERROR("wieg_init failed: %d"); }
    }
    else
    {
        ERROR("Configuration Error: Wiegand must be enabled");
        // TODO: Add rdm6300 lib
    }

    INFO("Setting up client");
    status = client_init(&config->client, config->device_type);
    client_handler_register(server_cmd_handler);

    INFO("Setting up authorized tag db");
    status = tags_init();
    if (status != STATUS_OK) { ERROR("tags_init failed: %d"); }

    switch(config->device_type) {
        case DEVICE_DOOR:
            INFO("Configuring as door");
            device = &door;
            break;

        case DEVICE_INTERLOCK:
            INFO("Configuring as interlock");
            device = &interlock;
            break;

        case DEVICE_VENDING:
            INFO("Configuring as vending");
            device = &vending;
            break;

        default:
            ERROR("Invalid device specified: %d.\nCheck configuration and reflash.");
    }

    INFO("Initializing device");
    status = device->init(config);
    if (status != STATUS_OK) { ERROR("device init failed: %d"); }

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static status_t server_cmd_handler(msg_t *msg)
{
    status_t status = -STATUS_INVALID;
    if (msg->type == MSG_REBOOT)
    {
        esp_restart();
        status = STATUS_OK;
    }
    if (msg->type == MSG_UPDATE_LOCKOUT)
    {
        WARN("Updating lockout setting to %u", msg->update_lockout.locked_out);
        nvstate_locked_out_set(msg->update_lockout.locked_out);
        status = STATUS_OK;
    }

    return status;
}

static int _reboot(int argc, char **argv)
{
    sys_restart();
    return 0;
}