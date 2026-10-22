#pragma once
#include "orbit_input.h"

#define ORBIT_POWER_DIM_US INT64_C(30000000)
#define ORBIT_POWER_OFF_US INT64_C(90000000)
#define ORBIT_POWER_BRIGHT 65
#define ORBIT_POWER_DIM 15

/* Caller serializes button edges and main-loop ticks. No hardware in callbacks. */
typedef struct {
    int64_t active_us;
    uint8_t pressed, wake_keys, brightness;
} orbit_power_t;

static inline bool orbit_power_edge(orbit_power_t *s, unsigned key,
                                    orbit_input_event_t event, int64_t now) {
    if (key >= 3) return false;
    uint8_t bit = (uint8_t)(1u << key);
    bool consume = (s->wake_keys & bit) != 0;
    if (event == ORBIT_INPUT_PRESS) {
        if (!s->brightness || s->wake_keys) {
            s->wake_keys |= bit;
            consume = true;
        }
        s->pressed |= bit;
        s->brightness = ORBIT_POWER_BRIGHT;
        s->active_us = now;
    } else if (event == ORBIT_INPUT_RELEASE) {
        s->pressed &= (uint8_t)~bit;
        s->wake_keys &= (uint8_t)~bit;
        s->active_us = now;
    }
    return consume;
}

static inline uint8_t orbit_power_tick(orbit_power_t *s, int64_t now, bool inhibit) {
    if (inhibit || s->pressed) s->active_us = now;
    int64_t idle = now - s->active_us;
    s->brightness = idle >= ORBIT_POWER_OFF_US ? 0 :
                    idle >= ORBIT_POWER_DIM_US ? ORBIT_POWER_DIM : ORBIT_POWER_BRIGHT;
    return s->brightness;
}
