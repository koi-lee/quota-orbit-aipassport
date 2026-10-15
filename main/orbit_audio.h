#pragma once
#include <stdbool.h>
#include "orbit_sound.h"
void orbit_audio_start(bool enabled);
void orbit_audio_enabled(bool enabled);
/* Bounded, non-blocking requests. Message cues take priority over local clicks. */
void orbit_audio_request(orbit_sound_cue_t cue);
