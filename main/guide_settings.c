#include "guide_settings.h"
uint8_t guide_settings_load_volume(nvs_handle_t handle, bool available) {
    uint8_t value=GUIDE_DEFAULT_VOLUME;
    if(!available || nvs_get_u8(handle,"volume",&value)!=ESP_OK || value>100) return GUIDE_DEFAULT_VOLUME;
    return value;
}
esp_err_t guide_settings_save_volume(nvs_handle_t handle, bool available, uint8_t volume) {
    if(!available || volume>100) return ESP_ERR_INVALID_STATE;
    esp_err_t error=nvs_set_u8(handle,"volume",volume);
    return error==ESP_OK?nvs_commit(handle):error;
}
