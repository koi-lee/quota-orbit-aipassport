#include "orbit_wish_model.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    orbit_wish_store_t s; uint8_t seed[32] = {1}, zero[32] = {0};
    orbit_wish_disable(&s); assert(orbit_wish_store_valid(&s)); assert(!orbit_wish_add(&s));
    assert(!orbit_wish_enable(&s, zero)); assert(!s.enabled);
    assert(orbit_wish_enable(&s, seed));
    for (unsigned i = 0; i < 20; ++i) assert(orbit_wish_add(&s));
    orbit_wish_store_t disk = s; /* Reopen the durable snapshot after power loss. */
    assert(orbit_wish_pending(&disk) == 20);
    assert(!orbit_wish_ack(&disk, 1, 20)); assert(!orbit_wish_ack(&disk, 0, 21));
    assert(orbit_wish_ack(&disk, 0, 20)); assert(!orbit_wish_ack(&disk, 0, 20));
    assert(orbit_wish_pending(&disk) == 0);
    for (unsigned i = 0; i < ORBIT_WISH_CAPACITY; ++i) assert(orbit_wish_add(&disk));
    assert(!orbit_wish_add(&disk)); assert(orbit_wish_ack(&disk, 20, ORBIT_WISH_BATCH));
    assert(orbit_wish_add(&disk)); orbit_wish_disable(&disk);
    assert(!orbit_wish_pending(&disk) && !disk.enabled && !disk.next);
    disk.enabled = 2; assert(!orbit_wish_store_valid(&disk));
    s.acknowledged = s.next + 1; assert(!orbit_wish_store_valid(&s));
    assert(orbit_wish_enable(&s, seed)); s.next = s.acknowledged = UINT64_MAX;
    assert(!orbit_wish_add(&s));
    puts("Wish opt-in, durable prefix, bounded queue, duplicate/stale ACK and sequence overflow PASS");
}
