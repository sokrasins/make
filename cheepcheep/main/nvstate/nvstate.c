#include "nvstate.h"
#include "log.h"

#include "nvs_flash.h"
#include "nvs.h"

#define NVS_LOCKED_OUT_KEY "locked_out"
#define NVS_TAG_HASH_KEY   "tag_hash"
#define NVS_TAG_CONFIG_KEY "config"

static nvs_handle_t _handle;

status_t nvstate_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open NVS handle
    INFO("Opening Non-Volatile Storage (NVS) handle...");
    err = nvs_open("storage", NVS_READWRITE, &_handle);
    if (err != ESP_OK) {
        ERROR("Error (%s) opening NVS handle!", esp_err_to_name(err));
        return -STATUS_IO;
    }
    
    nvs_type_t out_type;
    if (nvs_find_key(_handle, NVS_TAG_HASH_KEY, &out_type) == ESP_ERR_NVS_NOT_FOUND)
    {
        // initialize the key
        uint8_t tag_hash[16];
        memset(tag_hash, 0, 16);
        nvstate_tag_hash_set(tag_hash, 16);
    }

    if (nvs_find_key(_handle, NVS_LOCKED_OUT_KEY, &out_type) == ESP_ERR_NVS_NOT_FOUND)
    {
        // initialize the key
        INFO("Setting NVS key %s to default value", NVS_LOCKED_OUT_KEY);
        nvstate_locked_out_set(false);
    }
    
    return STATUS_OK;
}

bool nvstate_locked_out(void)
{
    uint8_t locked_out = 1;
    nvs_get_u8(_handle, NVS_LOCKED_OUT_KEY, &locked_out);
    return (bool) locked_out;
}

status_t nvstate_locked_out_set(bool locked_out)
{
    nvs_set_u8(_handle, NVS_LOCKED_OUT_KEY, (uint8_t) locked_out); 
    return STATUS_OK;
}

status_t nvstate_tag_hash(uint8_t *tag_hash, size_t *len)
{
    esp_err_t err = nvs_get_blob(_handle, NVS_TAG_HASH_KEY, (void *)tag_hash, len);
    return err == ESP_OK ? STATUS_OK : STATUS_NO_RESOURCE;
}

status_t nvstate_tag_hash_set(uint8_t *tag_hash, size_t len)
{
    nvs_set_blob(_handle, NVS_TAG_HASH_KEY, (void *)tag_hash, len);
    return STATUS_OK;
}

status_t nvstate_config(config_t *config)
{
    size_t bytes;
    esp_err_t err = nvs_get_blob(_handle, NVS_TAG_CONFIG_KEY, (void *)config, &bytes);
    return err == ESP_OK ? STATUS_OK : STATUS_NO_RESOURCE;
}

status_t nvstate_config_set(const config_t *config)
{
    esp_err_t err = nvs_set_blob(_handle, NVS_TAG_CONFIG_KEY, (void *)config, sizeof(config_t));
    return err == ESP_OK ? STATUS_OK : STATUS_NO_RESOURCE;
}
