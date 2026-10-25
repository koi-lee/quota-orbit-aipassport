#pragma once
#include "esp_err.h"
#include "nvs.h"
#include <stdbool.h>
#include <stdint.h>
#define GUIDE_DEFAULT_VOLUME 95
uint8_t guide_settings_load_volume(nvs_handle_t handle, bool available);
esp_err_t guide_settings_save_volume(nvs_handle_t handle, bool available, uint8_t volume);
