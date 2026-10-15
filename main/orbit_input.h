#pragma once
#include <stdbool.h>
#include <stdint.h>
/* Physical press/release edges, not delayed single/double-click classification. */
typedef enum { ORBIT_INPUT_PRESS, ORBIT_INPUT_RELEASE, ORBIT_INPUT_LONG, ORBIT_INPUT_OTHER } orbit_input_event_t;
typedef struct { uint8_t pressed, long_fired; } orbit_input_t;
static inline bool orbit_input_edge(orbit_input_t *s, unsigned key, orbit_input_event_t event,
                                    int *action) {
    if (key >= 3)
        return false;
    uint8_t bit = (uint8_t)(1u << key);
    if (event == ORBIT_INPUT_PRESS) {
        s->pressed |= bit;
        s->long_fired &= (uint8_t)~bit;
    } else if (event == ORBIT_INPUT_LONG && (s->pressed & bit) && !(s->long_fired & bit)) {
        s->long_fired |= bit;
        if (key == 2) {
            *action = 2;
            return true;
        }
    } else if (event == ORBIT_INPUT_RELEASE) {
        bool short_press = (s->pressed & bit) && !(s->long_fired & bit);
        s->pressed &= (uint8_t)~bit;
        s->long_fired &= (uint8_t)~bit;
        if (short_press) {
            const int actions[] = {-1, 1, 0};
            *action = actions[key];
            return true;
        }
    }
    return false;
}
