#include "esp_err.h"
#include "esp_log.h"
#include "esp_peripherals.h"
#include "input_key_service.h"
#include "periph_button.h"

#include "audio.h"
#include "config.h"
#include "shared.h"
#include "system.h"

static const char *TAG = "WILLOW/INPUT";

static esp_err_t cb_iks(periph_service_handle_t hdl, periph_service_event_t *ev, void *data)
{
    int key = (int)ev->data;
    int ret = ESP_OK;
    ESP_LOGD(TAG, "key pressed: type='%d', key='%d'", ev->type, key);

    if (key == INPUT_KEY_USER_ID_MUTE) {
        if (ev->type == INPUT_KEY_SERVICE_ACTION_PRESS_RELEASE) {
            ESP_LOGI(TAG, "unmute");
            init_adc();
        }
    }

    return ret;
}

esp_err_t init_buttons(void)
{
    periph_button_cfg_t cfg_btn = {
        .gpio_mask = ((1ULL << GPIO_NUM_0) | (1ULL << GPIO_NUM_1)), // BOOT/CONFIG | MUTE
    };
    esp_periph_handle_t hdl_btn = periph_button_init(&cfg_btn);
    if (hdl_btn == NULL) {
        return ESP_ERR_ADF_MEMORY_LACK;
    }
    return esp_periph_start(hdl_pset, hdl_btn);
}

esp_err_t init_input_key_service(void)
{
    int ret = ESP_OK;
    input_key_service_info_t inf_iks_boot = {
        .act_id = GPIO_NUM_0,
        .type = PERIPH_ID_BUTTON,
        .user_id = INPUT_KEY_USER_ID_REC,
    };
    input_key_service_info_t inf_iks_mute = {
        .act_id = GPIO_NUM_1,
        .type = PERIPH_ID_BUTTON,
        .user_id = INPUT_KEY_USER_ID_MUTE,
    };
    input_key_service_info_t inf_iks[] = {
        inf_iks_boot,
        inf_iks_mute,
    };

    input_key_service_cfg_t cfg_iks = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    cfg_iks.handle = hdl_pset;
    periph_service_handle_t hld_psvc_iks = input_key_service_create(&cfg_iks);
    ret = input_key_service_add_key(hld_psvc_iks, inf_iks, INPUT_KEY_NUM);
    if (ret != ESP_OK) {
        return ret;
    }
    return periph_service_set_callback(hld_psvc_iks, cb_iks, NULL);
}
