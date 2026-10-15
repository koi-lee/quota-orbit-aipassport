#pragma once
#include "orbit_net.h"
#include "orbit_quota_model.h"
#include "orbit_wish.h"
typedef struct {
    unsigned page, settings_row, stars, language;
    unsigned confirm_action;
    bool confirm_yes;
    bool settings, source, sound, pairing;
    orbit_quota_t quota;
    orbit_wish_view_t wish;
} orbit_ui_t;
void orbit_ui_draw(const orbit_ui_t *ui, const orbit_network_t *net, int battery, int64_t now);
/* up=-1, down=1, OK=0, long OK=2. Returns 1 provisioning, 2 sound, 7 unpair, 8 wish toggle, 9 wish click. */
int orbit_ui_key(orbit_ui_t *ui, int key);
