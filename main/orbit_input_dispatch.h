#pragma once
/* Called from the button timer: queue only, never draw or perform codec I/O. */
static inline bool orbit_input_dispatch(QueueHandle_t input, int action, volatile bool *click_context) {
    if (action != 0) *click_context = false;
    if (xQueueSend(input, &action, 0) != pdTRUE) return false;
    if (action == 0 && *click_context) orbit_audio_request(ORBIT_SOUND_CLICK);
    return true;
}
