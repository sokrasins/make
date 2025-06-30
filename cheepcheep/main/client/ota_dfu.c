#include "ota_dfu.h"
#include "log.h"
#include "net.h"
#include "config.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "esp_ota_ops.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define HTTP_CLIENT_TIMEOUT 5000 //ms
#define DFU_BUFFSIZE        1024

typedef struct {
    TaskHandle_t dfu_task_handle;
    net_evt_handle_t net_evt_handle;
    const config_dfu_t *config;
    char ota_write_data[DFU_BUFFSIZE + 1];
} dfu_ctx_t;

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void net_evt_cb(net_evt_t evt, void *ctx);
void dfu_task(void *params);

static dfu_ctx_t _ctx;

status_t ota_dfu_init(const config_dfu_t *config)
{
    _ctx.config = config;
    memset(_ctx.ota_write_data, 0, DFU_BUFFSIZE+1);

    if (strlen(_ctx.config->url) == 0)
    {
        INFO("No DFU server URL provided in the config, skipping check for new firmware");
        return STATUS_OK;
    }

    // Register connection CB with net, this will queue 
    // the dfu check after network connection
    _ctx.net_evt_handle = net_evt_cb_register(NET_EVT_CONNECT,(void *)&_ctx, net_evt_cb);

    return STATUS_OK;
}

status_t ota_mark_application(bool valid)
{
    esp_ota_img_states_t ota_state;
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) 
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) 
        {
            if (valid) 
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

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

void dfu_task(void *params)
{
    esp_err_t err;
    dfu_ctx_t * ctx = (dfu_ctx_t *) params;

    // update handle: set by esp_ota_begin(), must be freed via esp_ota_end()
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) 
    {
        WARN("Configured OTA boot partition at offset 0x%08"PRIx32", but running from offset 0x%08"PRIx32,
                 configured->address, running->address);
        WARN("(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    INFO("Running partition type %d subtype %d (offset 0x%08"PRIx32")", running->type, running->subtype, running->address);

    // Set client config
    esp_http_client_config_t config = {
        .url = ctx->config->url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
        .timeout_ms = HTTP_CLIENT_TIMEOUT,
    };

    if (ctx->config->skip_cn_check)
    {
        config.skip_cert_common_name_check = true;
    }

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) 
    {
        ERROR("Failed to initialise HTTP connection");
        goto kill_task;
    }

    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) 
    {
        WARN("Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        goto kill_task;
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    INFO("Writing to partition subtype %d at offset 0x%"PRIx32, update_partition->subtype, update_partition->address);

    int binary_file_length = 0;
    bool image_header_was_checked = false;

    while (1) 
    {
        int data_read = esp_http_client_read(client, ctx->ota_write_data, DFU_BUFFSIZE);
        if (data_read < 0) 
        {
            ERROR("Error: SSL data read error");
            http_cleanup(client);
            goto kill_task;
        } 
        else if (data_read > 0) 
        {
            if (image_header_was_checked == false) 
            {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) 
                {
                    // check current version with downloading
                    memcpy(&new_app_info, &ctx->ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    INFO("New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) 
                    {
                        INFO("Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) 
                    {
                        INFO("Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) 
                    {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) 
                        {
                            WARN("New version is the same as invalid version.");
                            WARN("Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            WARN("The firmware has been rolled back to the previous version.");
                            http_cleanup(client);
                            goto kill_task;
                        }
                    }

                    if (!ctx->config->skip_version_check)
                    {
                        if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) 
                        {
                            WARN("Current running version is the same as a new. We will not continue the update.");
                            http_cleanup(client);
                            goto kill_task;
                        }
                    }

                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                    if (err != ESP_OK) 
                    {
                        ERROR("esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(client);
                        esp_ota_abort(update_handle);
                        goto kill_task;
                    }
                    INFO("esp_ota_begin succeeded");
                } 
                else 
                {
                    ERROR("received package is not fit len");
                    http_cleanup(client);
                    esp_ota_abort(update_handle);
                    goto kill_task;
                }
            }
            
            err = esp_ota_write( update_handle, (const void *)ctx->ota_write_data, data_read);
            if (err != ESP_OK) 
            {
                http_cleanup(client);
                esp_ota_abort(update_handle);
                goto kill_task;
            }
            binary_file_length += data_read;
            INFO("Written image length %d", binary_file_length);
        } 
        else if (data_read == 0) 
        {
            // As esp_http_client_read never returns negative error code, we rely on
            // `errno` to check for underlying transport connectivity closure if any
            if (errno == ECONNRESET || errno == ENOTCONN) 
            {
                ERROR("Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) 
            {
                INFO("Connection closed");
                break;
            }
        }
    }
    INFO("Total Write binary data length: %d", binary_file_length);
    if (esp_http_client_is_complete_data_received(client) != true) 
    {
        ERROR("Error in receiving complete file");
        http_cleanup(client);
        esp_ota_abort(update_handle);
        goto kill_task;
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) 
    {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) 
        {
            ERROR("Image validation failed, image is corrupted");
        } 
        else 
        {
            ERROR("esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        http_cleanup(client);
        goto kill_task;    
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) 
    {
        ERROR("esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        goto kill_task;
    }
    INFO("Prepare to restart system!");
    esp_restart();

kill_task:
    // Kill the task, it is NOT allowed to return
    vTaskDelete(ctx->dfu_task_handle);
    while (1) { }
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        DEBUG("HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        DEBUG("HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        DEBUG("HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        DEBUG("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        DEBUG("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        DEBUG("HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        DEBUG("HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        DEBUG("HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}