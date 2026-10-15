#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "key_guard.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "orbit_ui.h"
#include "orbit_language.h"
#include "orbit_input.h"
#include "orbit_audio.h"
#include "orbit_quota.h"
#include "orbit_input_dispatch.h"
#include <string.h>
#include <time.h>

static QueueHandle_t input;
static key_guard_t guard;
static orbit_input_t key_edges;
static volatile bool s_input_ready;
static volatile bool s_click_sound_context;
static void on_key(bsp_btn_t key, bsp_btn_ev_t event, void *user) {
    (void)user;
    if (key_guard_consume(&guard, event == BSP_BTN_RELEASE, esp_timer_get_time())) {
        key_edges = (orbit_input_t){0};
        return;
    }
    if (!s_input_ready)
        return;
    orbit_input_event_t edge = event == BSP_BTN_PRESS ? ORBIT_INPUT_PRESS
                              : event == BSP_BTN_RELEASE ? ORBIT_INPUT_RELEASE
                              : event == BSP_BTN_LONG ? ORBIT_INPUT_LONG : ORBIT_INPUT_OTHER;
    int action;
    if (orbit_input_edge(&key_edges, (unsigned)key, edge, &action) && input)
        orbit_input_dispatch(input, action, &s_click_sound_context);
}
void app_main(void) {
    esp_err_t storage = nvs_flash_init();
    /* Never erase the user's shared NVS automatically on version/full errors. */
    if (storage != ESP_OK) {
        ESP_LOGE("orbit", "NVS unavailable: %s; no erase performed", esp_err_to_name(storage));
        return;
    }
    ESP_ERROR_CHECK(bsp_i2c_init());
    ESP_ERROR_CHECK(bsp_display_init());
    if (!bsp_lvgl_init())
        return;
    bsp_display_backlight(65);
    bool battery_ok = bsp_battery_init() == ESP_OK;
    int battery = battery_ok ? bsp_battery_soc() : -1;
    orbit_ui_t ui = {0};
    nvs_handle_t settings;
    bool have_settings = nvs_open("orbit_ui", NVS_READWRITE, &settings) == ESP_OK;
    if (have_settings) {
        uint8_t sound = 0;
        nvs_get_u8(settings, "sound", &sound);
        ui.sound = sound == 1;
        uint8_t language = 0;
        nvs_get_u8(settings, "language", &language);
        ui.language = language < ORBIT_LANGUAGE_COUNT ? language : ORBIT_ZH_CN;
    }
    input = xQueueCreate(8, sizeof(int));
    configASSERT(input);
    /* Suppress a held power-on key until release or the guard timeout. */
    key_guard_arm(&guard, esp_timer_get_time(), 1000000);
    ESP_ERROR_CHECK(bsp_button_init(on_key, NULL));
    key_guard_sync_released(&guard, bsp_button_read_mv() >= BSP_BTN_MV_RELEASED_MIN);
    orbit_net_start(ui.sound);
    orbit_quota_start();
    s_input_ready = true;
    orbit_network_t previous = {0};
    orbit_quota_t previous_quota = {0};
    bool previous_quota_fresh = false;
    int64_t last_minute = -1, last_battery = 0;
    bool dirty = true;
    for (;;) {
        int key;
        if (xQueueReceive(input, &key, pdMS_TO_TICKS(250)) == pdTRUE) {
            int action = orbit_ui_key(&ui, key);
            if (action == 3) orbit_quota_begin();
            if (action == 4) {
                if (ui.quota.paired) { orbit_quota_revoke(); ui.pairing = false; }
                else ui.pairing = !orbit_quota_confirm();
            }
            if (action == 5) orbit_quota_cancel();
            if (action == 1)
                orbit_net_provision();
            if (action == 2) {
                orbit_net_sound(ui.sound);
                if (have_settings) {
                    nvs_set_u8(settings, "sound", ui.sound);
                    nvs_commit(settings);
                }
                /* Finish Flash writes before starting the preview's PCM supply. */
                if (ui.sound) orbit_audio_request(ORBIT_SOUND_PREVIEW);
            }
            if (action == 6 && have_settings) {
                nvs_set_u8(settings, "language", ui.language);
                nvs_commit(settings);
            }
            if (action == 7) orbit_quota_revoke();
            dirty = true;
        }
        int64_t now = time(NULL), uptime = esp_timer_get_time() / 1000000;
        if (uptime - last_battery >= 60) {
            battery = battery_ok ? bsp_battery_soc() : -1;
            last_battery = uptime;
            dirty = true;
        }
        orbit_network_t net;
        orbit_net_read(&net);
        orbit_quota_read(&ui.quota);
        if (memcmp(&ui.quota, &previous_quota, sizeof(ui.quota))) dirty = true;
        if (orbit_quota_fresh(&ui.quota, now) != previous_quota_fresh) dirty = true;
        if (ui.pairing && !ui.quota.paired && !ui.quota.pairing) ui.pairing = false;
        s_click_sound_context = ui.page == 2 && !ui.settings && !ui.source && !ui.pairing && !net.setup;
        if (memcmp(&net, &previous, sizeof(net)) || now / 60 != last_minute)
            dirty = true;
        if (dirty && bsp_lvgl_lock(500)) {
            orbit_ui_draw(&ui, &net, battery, now);
            bsp_lvgl_unlock();
            previous = net;
            previous_quota = ui.quota;
            previous_quota_fresh = orbit_quota_fresh(&ui.quota, now);
            last_minute = now / 60;
            dirty = false;
        }
    }
}
