#include "ota_dfu.h"
#include "log.h"
#include "net.h"
#include "config.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "esp_ota_ops.h"
//#include "esp_app_format.h"
//#include "esp_http_client.h"
//#include "esp_flash_partitions.h"
//#include "esp_partition.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SKIP_COMMON_NAME_CHECK

typedef struct {
    TaskHandle_t dfu_task_handle;
    net_evt_handle_t net_evt_handle;
    const config_dfu_t *config;
} dfu_ctx_t;

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void net_evt_cb(net_evt_t evt, void *ctx);
void dfu_task(void *params);

static dfu_ctx_t _ctx;


#define BUFFSIZE 1024
static char ota_write_data[BUFFSIZE + 1] = { 0 };

status_t ota_dfu_init(const config_dfu_t *config)
{
    _ctx.config = config;

    // Register connection CB with net
    _ctx.net_evt_handle = net_evt_cb_register(NET_EVT_CONNECT,(void *)&_ctx, net_evt_cb);

    return STATUS_OK;
}

status_t ota_mark_application(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) 
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) 
        {
            // run diagnostic function ...
            bool diagnostic_is_ok = true; //diagnostic();
            if (diagnostic_is_ok) 
            {
                INFO("Diagnostics completed successfully! Continuing execution ...");
                esp_ota_mark_app_valid_cancel_rollback();
            } 
            else 
            {
                ERROR("Diagnostics failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }

    return STATUS_OK;
}

void net_evt_cb(net_evt_t evt, void *ctx)
{
    // Run net task, ready to handle network state changes
    dfu_ctx_t *dfu_ctx = (dfu_ctx_t *) ctx;
    xTaskCreate(dfu_task, "DFU_Task", 8192, ctx, 2, &dfu_ctx->dfu_task_handle);
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
    ERROR("Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void infinite_loop(void)
{
    int i = 0;
    INFO("When a new firmware is available on the server, press the reset button to download it");
    while(1) {
        INFO("Waiting for a new firmware ... %d", ++i);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void dfu_task(void *params)
{
    dfu_ctx_t * ctx = (dfu_ctx_t *) params;

    // Set config
    // event handler is set for future debugging
    esp_http_client_config_t config = {
        .url = ctx->config->url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
        .timeout_ms = 10000,
    };

#ifdef SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    INFO("Starting OTA example task");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        WARN("Configured OTA boot partition at offset 0x%08"PRIx32", but running from offset 0x%08"PRIx32,
                 configured->address, running->address);
        WARN("(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    INFO("Running partition type %d subtype %d (offset 0x%08"PRIx32")",
             running->type, running->subtype, running->address);

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ERROR("Failed to initialise HTTP connection");
        task_fatal_error();
    }
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ERROR("Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        task_fatal_error();
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    INFO("Writing to partition subtype %d at offset 0x%"PRIx32,
             update_partition->subtype, update_partition->address);

    int binary_file_length = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ERROR("Error: SSL data read error");
            http_cleanup(client);
            task_fatal_error();
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    INFO("New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        INFO("Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        INFO("Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            WARN("New version is the same as invalid version.");
                            WARN("Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            WARN("The firmware has been rolled back to the previous version.");
                            http_cleanup(client);
                            infinite_loop();
                        }
                    }
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        WARN("Current running version is the same as a new. We will not continue the update.");
                        http_cleanup(client);
                        infinite_loop();
                    }
#endif

                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                    if (err != ESP_OK) {
                        ERROR("esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(client);
                        esp_ota_abort(update_handle);
                        task_fatal_error();
                    }
                    INFO("esp_ota_begin succeeded");
                } else {
                    ERROR("received package is not fit len");
                    http_cleanup(client);
                    esp_ota_abort(update_handle);
                    task_fatal_error();
                }
            }
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                esp_ota_abort(update_handle);
                task_fatal_error();
            }
            binary_file_length += data_read;
            INFO("Written image length %d", binary_file_length);
        } else if (data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ERROR("Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {
                INFO("Connection closed");
                break;
            }
        }
    }
    INFO("Total Write binary data length: %d", binary_file_length);
    if (esp_http_client_is_complete_data_received(client) != true) {
        ERROR("Error in receiving complete file");
        http_cleanup(client);
        esp_ota_abort(update_handle);
        task_fatal_error();
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ERROR("Image validation failed, image is corrupted");
        } else {
            ERROR("esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        http_cleanup(client);
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ERROR("esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }
    INFO("Prepare to restart system!");
    esp_restart();

    // Kill the task, it is NOT allowed to return
    vTaskDelete(ctx->dfu_task_handle);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        INFO("HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        INFO("HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        INFO("HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        INFO("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        INFO("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        INFO("HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        INFO("HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        INFO("HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}