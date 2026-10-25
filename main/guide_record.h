#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "guide_data.h"
#define GUIDE_RECORD_SIZE 24
#define GUIDE_MASK ((UINT32_C(1) << GUIDE_SPOTS) - 1)
typedef struct { uint32_t seen, liked, sequence; uint8_t last; } guide_record_t;
void guide_record_encode(const guide_record_t *r, uint8_t out[GUIDE_RECORD_SIZE]);
bool guide_record_decode(guide_record_t *r, const uint8_t *data, size_t len);
/* Select the newest valid slot, retaining the other as recovery evidence. */
int guide_record_select(guide_record_t *r, const uint8_t *a, size_t na, const uint8_t *b, size_t nb);
/* Writer returns true only after the platform commit succeeds. */
typedef bool (*guide_record_writer_t)(void *context, unsigned slot, const uint8_t *data, size_t len);
bool guide_record_save(guide_record_t *r, int *slot, guide_record_writer_t writer, void *context);
