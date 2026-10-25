#pragma once
#include "guide_model.h"
#include <stdint.h>
typedef struct { guide_voice_state_t state; uint8_t spot; uint32_t generation; } guide_audio_status_t;
/* Only the audio worker owns BSP codec calls; requests never block the UI. */
bool guide_audio_start(void);
void guide_audio_play(uint8_t spot);
void guide_audio_stop(void);
void guide_audio_set_volume(uint8_t volume);
guide_audio_status_t guide_audio_status(void);
