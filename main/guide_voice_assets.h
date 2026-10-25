#pragma once
#include <stddef.h>
#include <stdint.h>
#include "guide_data.h"
typedef struct { const uint8_t *data; size_t bytes,samples; } guide_voice_clip_t;
extern const guide_voice_clip_t guide_voice_clips[GUIDE_SPOTS];
