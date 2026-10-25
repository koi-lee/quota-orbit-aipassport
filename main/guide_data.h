#pragma once
#include <stdint.h>
#define GUIDE_SPOTS 22
#define GUIDE_NONE 255
/* IDs are durable record keys; append new entries, never reorder existing IDs. */
typedef struct { const char *name, *highlight, *intro, *observe; uint8_t park, area, source; } guide_spot_t;
extern const guide_spot_t guide_spots[GUIDE_SPOTS];
extern const char *const guide_areas[6];
extern const char *const guide_parks[2];
