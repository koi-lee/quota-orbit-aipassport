#include "orbit_audio.h"
#include "bsp_audio.h"
#include "orbit_audio_device.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "orbit_audio_queue.h"
static QueueHandle_t messages, clicks;
static TaskHandle_t task;
static volatile bool enabled;
void orbit_audio_request(orbit_sound_cue_t cue) {
    if (!enabled || !task) return;
    if (!orbit_audio_enqueue(messages, clicks, cue, (uint32_t)(esp_timer_get_time() / 1000))) return;
    xTaskNotifyGive(task);
}
void orbit_audio_enabled(bool on) {
    enabled = on;
    if (!on && messages && clicks) { xQueueReset(messages); xQueueReset(clicks); }
    if (task) xTaskNotifyGive(task);
}
static bool play(orbit_sound_cue_t cue) {
    int16_t pcm[160];
    unsigned length = orbit_sound_playback_samples(cue);
    uint8_t volume = 70;
    for (unsigned base = 0; base < length && enabled; base += 160) {
        uint8_t next_volume = orbit_sound_output_volume(cue, base);
        if (volume != next_volume) { bsp_audio_set_volume(next_volume); volume = next_volume; }
        unsigned count = length - base < 160 ? length - base : 160;
        for (unsigned i = 0; i < count; i++) pcm[i] = orbit_sound_sample(cue, base + i);
        if (bsp_audio_write(pcm, count * sizeof(pcm[0])) != ESP_OK) {
            if (volume != 70) bsp_audio_set_volume(70);
            return false;
        }
    }
    if (volume != 70) bsp_audio_set_volume(70);
    return true;
}
static void audio_worker(void *arg) {
    (void)arg;
    bool awake = false;
    TickType_t last_activity = 0;
    unsigned measured = 0, stale = 0;
    uint32_t max_age_ms = 0, max_write_us = 0;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
        if (!enabled) { xQueueReset(messages); xQueueReset(clicks); }
        orbit_audio_event_t event;
        bool have = enabled && orbit_audio_dequeue(messages, clicks, &event, (uint32_t)(esp_timer_get_time() / 1000));
        if (have) {
            orbit_sound_cue_t cue = event.cue;
            last_activity = xTaskGetTickCount();
            if (!awake) {
                esp_err_t error = orbit_audio_device_ready();
                if (error != ESP_OK) {
                    ESP_LOGW("orbit_audio", "Audio preparation failed: %s; UI remains available", esp_err_to_name(error));
                    continue;
                }
                awake = true;
            }
            uint32_t age = (uint32_t)(esp_timer_get_time() / 1000) - event.requested_ms;
            if (!orbit_audio_event_fresh(&event, (uint32_t)(esp_timer_get_time() / 1000))) {
                ++stale;
                ESP_LOGI("orbit_audio", "Click expired after prepare: age_ms=%lu total=%u", (unsigned long)age, stale);
                if (uxQueueMessagesWaiting(messages) || uxQueueMessagesWaiting(clicks)) xTaskNotifyGive(task);
                continue;
            }
            int64_t started = esp_timer_get_time();
            if (!play(cue)) {
                ESP_LOGW("orbit_audio", "Audio write failed; retry on next cue");
                bsp_audio_sleep(); awake = false;
            } else if (cue != ORBIT_SOUND_CLICK) {
                ESP_LOGI("orbit_audio", "Cue played: %u", (unsigned)cue);
            }
            last_activity = xTaskGetTickCount();
            if (cue == ORBIT_SOUND_CLICK) {
                uint32_t written_us = (uint32_t)(esp_timer_get_time() - started);
                if (age > max_age_ms) max_age_ms = age;
                if (written_us > max_write_us) max_write_us = written_us;
                if (++measured == 5) {
                    ESP_LOGI("orbit_audio", "Click batch=5 max_start_age_ms=%lu max_write_us=%lu pending=%u",
                             (unsigned long)max_age_ms, (unsigned long)max_write_us, (unsigned)uxQueueMessagesWaiting(clicks));
                    measured = 0; max_age_ms = 0; max_write_us = 0;
                }
            }
            if (uxQueueMessagesWaiting(messages) || uxQueueMessagesWaiting(clicks)) xTaskNotifyGive(task);
        }
        if (awake && (!enabled || xTaskGetTickCount() - last_activity >= pdMS_TO_TICKS(3000))) {
            bsp_audio_sleep(); awake = false;
        }
    }
}
void orbit_audio_start(bool on) {
    enabled = on;
    messages = xQueueCreate(1, sizeof(orbit_audio_event_t));
    clicks = xQueueCreate(ORBIT_AUDIO_CLICK_CAPACITY, sizeof(orbit_audio_event_t));
    configASSERT(messages && clicks);
    /* PCM supply precedes taskLVGL (priority 4); writes/waits still block and yield. */
    configASSERT(xTaskCreate(audio_worker, "orbit_audio", 4096, NULL, 5, &task) == pdPASS);
}
