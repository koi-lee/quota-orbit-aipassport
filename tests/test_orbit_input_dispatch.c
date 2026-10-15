#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "../main/orbit_sound.h"
typedef void *QueueHandle_t;
#define pdTRUE 1
static unsigned queued, sounds;
static bool full;
static int xQueueSend(QueueHandle_t input, const int *action, int wait) {
    (void)input; (void)action; assert(wait == 0);
    if (full) return 0;
    ++queued; return 1;
}
static void orbit_audio_request(orbit_sound_cue_t cue) {
    assert(cue == ORBIT_SOUND_CLICK); assert(queued > sounds); ++sounds;
}
#include "../main/orbit_input_dispatch.h"
#include "../main/orbit_wish.h"
static unsigned wishes;
void orbit_wish_click(void) { ++wishes; }
int main(void) {
    volatile bool context = true;
    /* The UI consumer need not run for accepted physical clicks to request sound. */
    for (unsigned i = 0; i < 8; ++i) assert(orbit_input_dispatch(NULL, 0, &context));
    assert(sounds == 8);
    full = true; assert(!orbit_input_dispatch(NULL, 0, &context)); assert(sounds == 8);
    full = false;
    assert(orbit_input_dispatch(NULL, 2, &context) && !context);
    assert(orbit_input_dispatch(NULL, 0, &context)); assert(sounds == 8);
    context = true; assert(orbit_input_dispatch(NULL, 1, &context) && !context);
    assert(orbit_input_dispatch(NULL, 0, &context)); assert(sounds == 8);
    /* A later navigation/long press can invalidate audio before the UI consumes
       an earlier accepted tap. It must not erase that tap's upload intention. */
    context = true;
    assert(orbit_input_dispatch(NULL, 0, &context));
    assert(orbit_input_dispatch(NULL, 2, &context) && !context);
    orbit_wish_dispatch(9, false);
    assert(wishes == 1);
    orbit_wish_dispatch(0, false); orbit_wish_dispatch(8, false);
    orbit_wish_dispatch(9, true);
    assert(wishes == 1);
    puts("Audio requested before UI consumption; full queue, navigation and long-OK stay silent PASS");
}
