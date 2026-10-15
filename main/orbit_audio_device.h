#pragma once

/* The caller supplies BSP declarations. init alone does not wake a sleeping codec. */
static inline esp_err_t orbit_audio_device_ready(void) {
    esp_err_t error = bsp_audio_init();
    if (error == ESP_OK) error = bsp_audio_wake();
    if (error == ESP_OK) error = bsp_audio_set_format(16000, 16, 1);
    if (error == ESP_OK) bsp_audio_set_volume(70);
    return error;
}
