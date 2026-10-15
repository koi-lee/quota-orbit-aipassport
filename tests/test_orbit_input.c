#include "orbit_input.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    orbit_input_t s = {0};
    int action = -99;
    /* Every physical short press counts, even if the driver later reports a double click. */
    for (unsigned i = 0; i < 30; i++) {
        assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_PRESS, &action));
        assert(orbit_input_edge(&s, 2, ORBIT_INPUT_RELEASE, &action) && action == 0);
        assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_OTHER, &action));
        assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_RELEASE, &action));
    }
    /* Long OK opens settings once, and release cannot also trigger a short click. */
    assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_PRESS, &action));
    assert(orbit_input_edge(&s, 2, ORBIT_INPUT_LONG, &action) && action == 2);
    assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_LONG, &action));
    assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_RELEASE, &action));
    for (unsigned key = 0; key < 2; key++) {
        assert(!orbit_input_edge(&s, key, ORBIT_INPUT_PRESS, &action));
        assert(orbit_input_edge(&s, key, ORBIT_INPUT_RELEASE, &action));
        assert(action == (key == 0 ? -1 : 1));
    }
    /* Suppressed startup edges, unmatched release and invalid keys do not create actions. */
    s = (orbit_input_t){0};
    assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_RELEASE, &action));
    assert(!orbit_input_edge(&s, 2, ORBIT_INPUT_LONG, &action));
    assert(!orbit_input_edge(&s, 9, ORBIT_INPUT_PRESS, &action));
    puts("Immediate short releases, rapid taps, no duplicate clicks and long-OK settings PASS");
    return 0;
}
