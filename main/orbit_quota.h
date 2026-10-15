#pragma once
#include "orbit_quota_model.h"
void orbit_quota_start(void);
void orbit_quota_read(orbit_quota_t *out);
void orbit_quota_begin(void);
bool orbit_quota_confirm(void);
void orbit_quota_cancel(void);
void orbit_quota_revoke(void);
