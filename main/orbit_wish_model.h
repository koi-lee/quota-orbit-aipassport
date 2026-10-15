#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define ORBIT_WISH_CAPACITY 10000u
#define ORBIT_WISH_BATCH 32u
#define ORBIT_WISH_VERSION 1u
typedef struct {
    uint32_t version, enabled;
    uint64_t acknowledged, next;
    uint8_t seed[32];
} orbit_wish_store_t;
static inline bool orbit_wish_store_valid(const orbit_wish_store_t *s) {
    if (s->version != ORBIT_WISH_VERSION || s->enabled > 1 || s->next < s->acknowledged ||
        s->next - s->acknowledged > ORBIT_WISH_CAPACITY) return false;
    if (!s->enabled) return s->next == 0 && s->acknowledged == 0;
    unsigned nonzero = 0;
    for (unsigned i = 0; i < 32; ++i) nonzero |= s->seed[i];
    return nonzero != 0;
}
static inline unsigned orbit_wish_pending(const orbit_wish_store_t *s) {
    return s->enabled ? (unsigned)(s->next - s->acknowledged) : 0;
}
static inline void orbit_wish_disable(orbit_wish_store_t *s) {
    memset(s, 0, sizeof(*s)); s->version = ORBIT_WISH_VERSION;
}
static inline bool orbit_wish_enable(orbit_wish_store_t *s, const uint8_t seed[32]) {
    orbit_wish_disable(s); memcpy(s->seed, seed, 32); s->enabled = 1;
    if (orbit_wish_store_valid(s)) return true;
    orbit_wish_disable(s); return false;
}
static inline bool orbit_wish_add(orbit_wish_store_t *s) {
    if (!orbit_wish_store_valid(s) || !s->enabled || orbit_wish_pending(s) >= ORBIT_WISH_CAPACITY || s->next == UINT64_MAX)
        return false;
    ++s->next; return true;
}
/* A response can only acknowledge the exact prefix sent from this snapshot. */
static inline bool orbit_wish_ack(orbit_wish_store_t *s, uint64_t first, unsigned count) {
    if (!s->enabled || !count || count > ORBIT_WISH_BATCH || first != s->acknowledged ||
        count > orbit_wish_pending(s)) return false;
    s->acknowledged += count; return true;
}
