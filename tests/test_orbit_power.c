#include <assert.h>
#include <stdio.h>
#include "orbit_power.h"
int main(void) {
    orbit_power_t s = {.brightness = ORBIT_POWER_BRIGHT};
    assert(orbit_power_tick(&s, ORBIT_POWER_DIM_US - 1, false) == 65);
    assert(orbit_power_tick(&s, ORBIT_POWER_DIM_US, false) == 15);
    assert(orbit_power_tick(&s, ORBIT_POWER_OFF_US - 1, false) == 15);
    assert(orbit_power_tick(&s, ORBIT_POWER_OFF_US, false) == 0);
    /* Every key, including long OK, wakes without navigation or a wish. */
    for (unsigned key = 0; key < 3; ++key) {
        s = (orbit_power_t){0};
        orbit_input_t input = {0};
        int action = 99;
        orbit_input_event_t events[] = {ORBIT_INPUT_PRESS, ORBIT_INPUT_LONG, ORBIT_INPUT_RELEASE};
        for (unsigned i = 0; i < 3; ++i) {
            bool consumed = orbit_power_edge(&s, key, events[i], 100 + i);
            assert(consumed);
            if (!consumed) orbit_input_edge(&input, key, events[i], &action);
        }
        assert(action == 99 && !input.pressed && !s.pressed && !s.wake_keys);
        assert(orbit_power_tick(&s, 103, false) == 65);
        assert(!orbit_power_edge(&s, key, ORBIT_INPUT_PRESS, 104));
        assert(!orbit_input_edge(&input, key, ORBIT_INPUT_PRESS, &action));
        assert(!orbit_power_edge(&s, key, ORBIT_INPUT_RELEASE, 105));
        assert(orbit_input_edge(&input, key, ORBIT_INPUT_RELEASE, &action));
        assert(action == (key == 0 ? -1 : key == 1 ? 1 : 0));
    }
    /* Dimmed presses still act normally, and held keys never time out. */
    s = (orbit_power_t){.brightness = 15};
    assert(!orbit_power_edge(&s, 2, ORBIT_INPUT_PRESS, 1));
    assert(orbit_power_tick(&s, ORBIT_POWER_OFF_US * 2, false) == 65);
    assert(!orbit_power_edge(&s, 2, ORBIT_INPUT_RELEASE, ORBIT_POWER_OFF_US * 2 + 1));
    assert(orbit_power_tick(&s, ORBIT_POWER_OFF_US * 2 + 2, false) == 65);
    /* Provisioning/pairing keeps the screen visible, then grants a full timeout. */
    s = (orbit_power_t){0};
    assert(orbit_power_tick(&s, ORBIT_POWER_OFF_US * 3, true) == 65);
    assert(orbit_power_tick(&s, ORBIT_POWER_OFF_US * 3 + 1, false) == 65);
    assert(orbit_power_tick(&s, ORBIT_POWER_OFF_US * 4, false) == 0);
    /* Suppress an overlapping second wake key through both releases. */
    assert(orbit_power_edge(&s, 0, ORBIT_INPUT_PRESS, ORBIT_POWER_OFF_US * 4 + 1));
    assert(orbit_power_edge(&s, 2, ORBIT_INPUT_PRESS, ORBIT_POWER_OFF_US * 4 + 2));
    assert(orbit_power_edge(&s, 0, ORBIT_INPUT_RELEASE, ORBIT_POWER_OFF_US * 4 + 3));
    assert(orbit_power_edge(&s, 2, ORBIT_INPUT_LONG, ORBIT_POWER_OFF_US * 4 + 4));
    assert(orbit_power_edge(&s, 2, ORBIT_INPUT_RELEASE, ORBIT_POWER_OFF_US * 4 + 5));
    assert(!s.pressed && !s.wake_keys);
    puts("orbit power: PASS");
}
