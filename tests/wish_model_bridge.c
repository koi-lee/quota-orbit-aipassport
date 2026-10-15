/* Host integration bridge: tests drive the same queue model as the firmware. */
#include "orbit_wish_model.h"
#include <stddef.h>
size_t wish_size(void) { return sizeof(orbit_wish_store_t); }
int wish_enable(void *p, const uint8_t *seed) { return orbit_wish_enable(p, seed); }
int wish_add(void *p) { return orbit_wish_add(p); }
void wish_disable(void *p) { orbit_wish_disable(p); }
unsigned wish_pending(void *p) { return orbit_wish_pending(p); }
uint64_t wish_first(void *p) { return ((orbit_wish_store_t *)p)->acknowledged; }
int wish_ack(void *p, uint64_t first, unsigned count) { return orbit_wish_ack(p, first, count); }
