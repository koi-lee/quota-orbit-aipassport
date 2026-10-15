#pragma once
#include <stdbool.h>
typedef enum {
    WISH_OFF, WISH_READY, WISH_PENDING, WISH_SYNCED, WISH_OFFLINE,
    WISH_STORAGE_ERROR, WISH_FULL, WISH_RETRY, WISH_UNAVAILABLE
} orbit_wish_status_t;
typedef struct {
    bool enabled, ready;
    unsigned pending;
    orbit_wish_status_t status;
} orbit_wish_view_t;
void orbit_wish_start(void);
void orbit_wish_read(orbit_wish_view_t *out);
void orbit_wish_set_enabled(bool enabled);
void orbit_wish_click(void);
