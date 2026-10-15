#pragma once
#define ORBIT_AUDIO_CLICK_CAPACITY 4
#define ORBIT_AUDIO_CLICK_MAX_AGE_MS 120
typedef struct { orbit_sound_cue_t cue; uint32_t requested_ms; } orbit_audio_event_t;
/* The caller supplies FreeRTOS queue declarations and orbit_sound_cue_t. */
static inline bool orbit_audio_enqueue(QueueHandle_t messages, QueueHandle_t clicks, orbit_sound_cue_t cue, uint32_t now) {
    orbit_audio_event_t event = {cue, now};
    if (cue != ORBIT_SOUND_CLICK) return xQueueOverwrite(messages, &event) == pdTRUE;
    if (xQueueSend(clicks, &event, 0) == pdTRUE) return true;
    /* Under overload retain the new physical tap rather than a delayed old tap. */
    orbit_audio_event_t discarded;
    xQueueReceive(clicks, &discarded, 0);
    return xQueueSend(clicks, &event, 0) == pdTRUE;
}
static inline bool orbit_audio_event_fresh(const orbit_audio_event_t *event, uint32_t now) {
    return event->cue != ORBIT_SOUND_CLICK || now - event->requested_ms <= ORBIT_AUDIO_CLICK_MAX_AGE_MS;
}
static inline bool orbit_audio_dequeue(QueueHandle_t messages, QueueHandle_t clicks, orbit_audio_event_t *event, uint32_t now) {
    if (xQueueReceive(messages, event, 0) == pdTRUE) return true;
    while (xQueueReceive(clicks, event, 0) == pdTRUE) if (orbit_audio_event_fresh(event, now)) return true;
    return false;
}
