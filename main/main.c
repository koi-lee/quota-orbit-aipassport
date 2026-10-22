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
#include "orbit_power.h"
#include "orbit_audio.h"
#include "orbit_quota.h"
#include "orbit_input_dispatch.h"
#include <string.h>
#include <time.h>

static QueueHandle_t input;
static key_guard_t guard;
static orbit_input_t key_edges;
static orbit_power_t power = {.brightness = ORBIT_POWER_BRIGHT};
static portMUX_TYPE power_lock = portMUX_INITIALIZER_UNLOCKED;
static volatile bool s_input_ready;
static volatile bool s_click_sound_context;
/* Bounded, anonymous counters distinguish driver edges from queue/UI stalls. */
static portMUX_TYPE input_metrics_lock = portMUX_INITIALIZER_UNLOCKED;
static struct {
    uint32_t press, release, queued, dropped, consumed, taps, draws, lock_fail;
    uint32_t max_queue, max_draw_us, max_loop_us;
} input_metrics;
static void report_input_metrics(void) {
    static int64_t last_report;
    static uint32_t last_edges;
    int64_t now = esp_timer_get_time();
    if (now - last_report < 2000000) return;
    last_report = now;
    taskENTER_CRITICAL(&input_metrics_lock);
    uint32_t edges = input_metrics.press + input_metrics.release;
    uint32_t press = input_metrics.press, release = input_metrics.release;
    uint32_t queued = input_metrics.queued, dropped = input_metrics.dropped;
    uint32_t consumed = input_metrics.consumed, taps = input_metrics.taps;
    uint32_t draws = input_metrics.draws, lock_fail = input_metrics.lock_fail;
    uint32_t max_queue = input_metrics.max_queue, draw_us = input_metrics.max_draw_us;
    uint32_t loop_us = input_metrics.max_loop_us;
    taskEXIT_CRITICAL(&input_metrics_lock);
    if (edges == last_edges) return;
    last_edges = edges;
    ESP_LOGI("orbit_input", "press=%lu release=%lu queued=%lu dropped=%lu consumed=%lu taps=%lu draws=%lu lock_fail=%lu queue_peak=%lu draw_max_us=%lu loop_max_us=%lu",
             (unsigned long)press, (unsigned long)release, (unsigned long)queued,
             (unsigned long)dropped, (unsigned long)consumed, (unsigned long)taps,
             (unsigned long)draws, (unsigned long)lock_fail, (unsigned long)max_queue,
             (unsigned long)draw_us, (unsigned long)loop_us);
}
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
    taskENTER_CRITICAL(&power_lock);
    bool wake_only = orbit_power_edge(&power, (unsigned)key, edge, esp_timer_get_time());
    taskEXIT_CRITICAL(&power_lock);
    if (wake_only) return;
    taskENTER_CRITICAL(&input_metrics_lock);
    if (key == BSP_BTN_OK && edge == ORBIT_INPUT_PRESS) ++input_metrics.press;
    if (key == BSP_BTN_OK && edge == ORBIT_INPUT_RELEASE) ++input_metrics.release;
    taskEXIT_CRITICAL(&input_metrics_lock);
    int action;
    if (orbit_input_edge(&key_edges, (unsigned)key, edge, &action) && input) {
        bool queued = orbit_input_dispatch(input, action, &s_click_sound_context);
        unsigned depth = uxQueueMessagesWaiting(input);
        taskENTER_CRITICAL(&input_metrics_lock);
        if (queued) ++input_metrics.queued;
        else ++input_metrics.dropped;
        if (depth > input_metrics.max_queue) input_metrics.max_queue = depth;
        taskEXIT_CRITICAL(&input_metrics_lock);
    }
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
    orbit_ui_t ui = {.sound = true, .language = ORBIT_ZH_CN};
    nvs_handle_t settings;
    bool have_settings = nvs_open("orbit_ui", NVS_READWRITE, &settings) == ESP_OK;
    if (have_settings) {
        uint8_t sound = 1;
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
    orbit_wish_start();
    /* Match taskLVGL (4), below PCM (5): unlocking LVGL must not starve input. */
    vTaskPrioritySet(NULL, 4);
    power.active_us = esp_timer_get_time();
    uint8_t brightness = ORBIT_POWER_BRIGHT;
    s_input_ready = true;
    orbit_network_t previous = {0};
    orbit_quota_t previous_quota = {0};
    bool previous_quota_fresh = false;
    int64_t last_minute = -1, last_battery = 0;
    bool dirty = true;
    for (;;) {
        int key;
        int64_t loop_started = esp_timer_get_time();
        if (xQueueReceive(input, &key, pdMS_TO_TICKS(250)) == pdTRUE) {
            unsigned handled = 0;
            do {
                int action = orbit_ui_key(&ui, key);
                taskENTER_CRITICAL(&input_metrics_lock);
                ++input_metrics.consumed;
                if (action == 9) ++input_metrics.taps;
                taskEXIT_CRITICAL(&input_metrics_lock);
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
                if (action == 8) orbit_wish_set_enabled(!ui.wish.enabled);
                orbit_network_t input_net;
                orbit_net_read(&input_net);
                orbit_wish_dispatch(action, input_net.setup);
                dirty = true;
                /* Coalesce rendering only. Every tap keeps its count and upload;
                   never consume navigation/settings out of order. */
                int next_key;
                if (++handled >= 8 || action != 9 ||
                    xQueuePeek(input, &next_key, 0) != pdTRUE || next_key != 0)
                    break;
            } while (xQueueReceive(input, &key, 0) == pdTRUE);
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
        orbit_wish_view_t wish;
        orbit_wish_read(&wish);
        if (memcmp(&ui.wish, &wish, sizeof(wish))) dirty = true;
        ui.wish = wish;
        if (memcmp(&ui.quota, &previous_quota, sizeof(ui.quota))) dirty = true;
        if (orbit_quota_fresh(&ui.quota, now) != previous_quota_fresh) dirty = true;
        if (ui.pairing && !ui.quota.paired && !ui.quota.pairing) ui.pairing = false;
        s_click_sound_context = ui.page == 2 && !ui.settings && !ui.source && !ui.pairing && !net.setup;
        if (memcmp(&net, &previous, sizeof(net)) || now / 60 != last_minute)
            dirty = true;
        taskENTER_CRITICAL(&power_lock);
        uint8_t next_brightness = orbit_power_tick(&power, esp_timer_get_time(),
                                                  net.setup || ui.pairing || ui.quota.pairing);
        taskEXIT_CRITICAL(&power_lock);
        if (next_brightness != brightness) {
            bsp_display_backlight(next_brightness);
            ESP_LOGI("orbit_power", "Backlight %u -> %u", brightness, next_brightness);
            if (!brightness && next_brightness) dirty = true;
            brightness = next_brightness;
        }
        /* Keep networking alive; defer page rebuilds until the display is visible. */
        if (dirty && brightness && bsp_lvgl_lock(500)) {
            int64_t draw_started = esp_timer_get_time();
            orbit_ui_draw(&ui, &net, battery, now);
            uint32_t elapsed = (uint32_t)(esp_timer_get_time() - draw_started);
            bsp_lvgl_unlock();
            ++input_metrics.draws;
            if (elapsed > input_metrics.max_draw_us) input_metrics.max_draw_us = elapsed;
            previous = net;
            previous_quota = ui.quota;
            previous_quota_fresh = orbit_quota_fresh(&ui.quota, now);
            last_minute = now / 60;
            dirty = false;
        } else if (dirty && brightness) {
            ++input_metrics.lock_fail;
        }
        uint32_t loop_us = (uint32_t)(esp_timer_get_time() - loop_started);
        if (loop_us > input_metrics.max_loop_us) input_metrics.max_loop_us = loop_us;
        report_input_metrics();
    }
}
