#pragma once
#include <stddef.h>
#include <stdint.h>
typedef struct { int predictor,index; size_t position; } guide_adpcm_t;
/* Continuous IMA/DVI nibbles, low nibble first; each clip starts at state zero. */
size_t guide_adpcm_decode(guide_adpcm_t *s,const uint8_t *data,size_t bytes,size_t samples,int16_t *out,size_t capacity);
