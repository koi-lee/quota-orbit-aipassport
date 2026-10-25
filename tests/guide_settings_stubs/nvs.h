#pragma once
#include "esp_err.h"
#include <stdint.h>
typedef unsigned nvs_handle_t;
esp_err_t nvs_get_u8(nvs_handle_t, const char *, uint8_t *);
esp_err_t nvs_set_u8(nvs_handle_t, const char *, uint8_t);
esp_err_t nvs_commit(nvs_handle_t);
