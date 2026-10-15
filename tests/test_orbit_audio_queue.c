#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../main/orbit_sound.h"
typedef struct { unsigned char items[4][8]; unsigned length, capacity; } queue_t;
typedef queue_t *QueueHandle_t;
#define pdTRUE 1
static int xQueueSend(QueueHandle_t q, const void *event, int wait) {
    assert(wait == 0);
    if (q->length == q->capacity) return 0;
    memcpy(q->items[q->length++], event, 8); return 1;
}
static int xQueueOverwrite(QueueHandle_t q, const void *event) {
    assert(q->capacity == 1); memcpy(q->items[0], event, 8); q->length = 1; return 1;
}
static int xQueueReceive(QueueHandle_t q, void *event, int wait) {
    assert(wait == 0);
    if (!q->length) return 0;
    memcpy(event, q->items[0], 8);
    for (unsigned i = 1; i < q->length; ++i) memcpy(q->items[i - 1], q->items[i], 8);
    --q->length; return 1;
}
#include "../main/orbit_audio_queue.h"
int main(void) {
    _Static_assert(sizeof(orbit_audio_event_t) == 8, "mock item size");
    queue_t messages = {.capacity = 1}, clicks = {.capacity = ORBIT_AUDIO_CLICK_CAPACITY};
    orbit_audio_event_t event;
    /* Overload retains four newest taps, not a long tail of old feedback. */
    for (unsigned i = 0; i < 10; ++i) assert(orbit_audio_enqueue(&messages, &clicks, ORBIT_SOUND_CLICK, i * 10));
    assert(orbit_audio_enqueue(&messages, &clicks, ORBIT_SOUND_FORECAST, 0));
    assert(orbit_audio_enqueue(&messages, &clicks, ORBIT_SOUND_COMPLETE, 0));
    assert(orbit_audio_dequeue(&messages, &clicks, &event, 100) && event.cue == ORBIT_SOUND_COMPLETE);
    for (unsigned i = 6; i < 10; ++i) {
        assert(orbit_audio_dequeue(&messages, &clicks, &event, 100));
        assert(event.cue == ORBIT_SOUND_CLICK && event.requested_ms == i * 10);
    }
    assert(!orbit_audio_dequeue(&messages, &clicks, &event, 100));
    for (unsigned i = 0; i < 100; ++i) {
        assert(orbit_audio_enqueue(&messages, &clicks, ORBIT_SOUND_CLICK, i * 50));
        assert(orbit_audio_dequeue(&messages, &clicks, &event, i * 50 + 24));
        assert(event.cue == ORBIT_SOUND_CLICK);
    }
    assert(orbit_audio_enqueue(&messages, &clicks, ORBIT_SOUND_CLICK, 6000));
    assert(!orbit_audio_dequeue(&messages, &clicks, &event, 6121));
    assert(orbit_audio_enqueue(&messages, &clicks, ORBIT_SOUND_CLICK, UINT32_MAX - 20));
    assert(orbit_audio_dequeue(&messages, &clicks, &event, 30));
    assert(orbit_audio_event_fresh(&event, 30));
    assert(!orbit_audio_event_fresh(&event, 200));
    assert(orbit_audio_enqueue(&messages, &clicks, ORBIT_SOUND_PREVIEW, 0));
    assert(orbit_audio_dequeue(&messages, &clicks, &event, 9000) && event.cue == ORBIT_SOUND_PREVIEW);
    puts("Fresh click routing, newest bounded overload, expiry, clock wrap and message priority PASS");
}
