#pragma once
#include "orbit_model.h"
typedef enum {
    NET_CONNECTING,
    NET_SYNCING,
    NET_ONLINE,
    NET_OFFLINE,
    NET_CLOCK,
    NET_SETUP,
    NET_BAD_WIFI,
    NET_SETUP_EXPIRED,
    NET_ERROR
} orbit_link_t;
typedef struct {
    orbit_snapshot_t snapshot;
    orbit_link_t link;
    unsigned wifi_reason;
    bool setup, sound, new_message;
    char ap_name[24], ap_password[16];
    char notification_test[48];
} orbit_network_t;
void orbit_net_start(bool sound);
void orbit_net_read(orbit_network_t *out);
void orbit_net_provision(void);
void orbit_net_sound(bool enabled);
