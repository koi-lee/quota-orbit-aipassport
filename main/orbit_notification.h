#pragma once
#include "orbit_model.h"
#include "orbit_sound.h"
typedef struct { bool new_message, play; orbit_sound_cue_t cue; } orbit_notification_t;
static inline orbit_notification_t orbit_notification_plan(const orbit_snapshot_t *old,
    const orbit_snapshot_t *next, int64_t now, bool sound) {
    bool notify = orbit_should_notify(old, next, now);
    return (orbit_notification_t){.new_message = notify, .play = notify && sound,
        .cue = next->kind == ORBIT_COMPLETED ? ORBIT_SOUND_COMPLETE : ORBIT_SOUND_FORECAST};
}
