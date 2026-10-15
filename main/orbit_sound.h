#pragma once
#include <stdint.h>
typedef enum { ORBIT_SOUND_CLICK, ORBIT_SOUND_FORECAST, ORBIT_SOUND_COMPLETE, ORBIT_SOUND_PREVIEW } orbit_sound_cue_t;
typedef struct { uint16_t hz, ms; } orbit_sound_note_t;
static inline const orbit_sound_note_t *orbit_sound_notes(orbit_sound_cue_t cue, unsigned *count) {
    static const orbit_sound_note_t click[] = {{1108, 80}};
    static const orbit_sound_note_t forecast[] = {{1108, 90}, {0, 25}, {1320, 110}};
    static const orbit_sound_note_t complete[] = {{1046, 80}, {0, 25}, {1108, 80}, {0, 25}, {1320, 120}};
    /* Play the actual click, forecast and completion cues, separated by silence. */
    static const orbit_sound_note_t preview[] = {{1108, 80}, {0, 350}, {1108, 90}, {0, 25},
        {1320, 110}, {0, 350}, {1046, 80}, {0, 25}, {1108, 80}, {0, 25}, {1320, 120}};
    switch (cue) {
    case ORBIT_SOUND_CLICK: *count = 1; return click;
    case ORBIT_SOUND_FORECAST: *count = 3; return forecast;
    case ORBIT_SOUND_COMPLETE: *count = 5; return complete;
    case ORBIT_SOUND_PREVIEW: *count = 11; return preview;
    }
    *count = 0; return click;
}
static inline unsigned orbit_sound_samples(orbit_sound_cue_t cue) {
    unsigned count, samples = 0;
    const orbit_sound_note_t *notes = orbit_sound_notes(cue, &count);
    for (unsigned i = 0; i < count; i++) samples += notes[i].ms * 16u;
    return samples;
}
static inline uint8_t orbit_sound_output_volume(orbit_sound_cue_t cue, unsigned sample) {
    (void)cue; (void)sample;
    return 70;
}
/* Supply complete 10ms blocks plus a 20ms silent tail before the next cue. */
static inline unsigned orbit_sound_playback_samples(orbit_sound_cue_t cue) {
    return ((orbit_sound_samples(cue) + 159u) / 160u) * 160u + 320u;
}
static inline int16_t orbit_sound_sample(orbit_sound_cue_t cue, unsigned sample) {
    static const int16_t sine[] = {0, 3196, 6270, 9102, 11585, 13623, 15137, 16069,
        16384, 16069, 15137, 13623, 11585, 9102, 6270, 3196, 0, -3196, -6270, -9102,
        -11585, -13623, -15137, -16069, -16384, -16069, -15137, -13623, -11585, -9102, -6270, -3196};
    unsigned count;
    const orbit_sound_note_t *notes = orbit_sound_notes(cue, &count);
    for (unsigned i = 0; i < count; i++) {
        unsigned length = notes[i].ms * 16u;
        if (sample >= length) { sample -= length; continue; }
        if (!notes[i].hz) return 0;
        unsigned ramp = (cue == ORBIT_SOUND_CLICK || (cue == ORBIT_SOUND_PREVIEW && i == 0)) ? 256u : 128u;
        unsigned envelope = sample < ramp ? sample : length - 1 - sample < ramp ? length - 1 - sample : ramp;
        uint32_t phase = (uint32_t)(((uint64_t)sample * notes[i].hz << 32) / 16000);
        unsigned index = phase >> 27;
        unsigned fraction = (phase & 0x07ffffffu) >> 15;
        int32_t smooth = sine[index] + ((int32_t)sine[(index + 1) & 31] - sine[index]) * (int32_t)fraction / 4096;
        /* Smoothstep has zero slope at both ends, avoiding the linear envelope's corner. */
        int32_t fade = (int32_t)((uint64_t)envelope * envelope * (3 * ramp - 2 * envelope) * 32768u / ((uint64_t)ramp * ramp * ramp));
        return (int16_t)((smooth * fade / 32768) * 5400 / 16384);
    }
    return 0;
}
