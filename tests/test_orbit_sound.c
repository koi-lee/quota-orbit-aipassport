#include "orbit_sound.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main(int argc, char **argv) {
    assert(orbit_sound_samples(ORBIT_SOUND_CLICK) == 1280);
    assert(orbit_sound_samples(ORBIT_SOUND_FORECAST) != orbit_sound_samples(ORBIT_SOUND_COMPLETE));
    for (unsigned cue = 0; cue <= ORBIT_SOUND_PREVIEW; cue++) {
        unsigned size = orbit_sound_samples((orbit_sound_cue_t)cue);
        assert(size && orbit_sound_sample((orbit_sound_cue_t)cue, 0) == 0);
        assert(orbit_sound_sample((orbit_sound_cue_t)cue, size - 1) == 0);
        assert(orbit_sound_sample((orbit_sound_cue_t)cue, size) == 0);
        unsigned playback = orbit_sound_playback_samples((orbit_sound_cue_t)cue);
        assert(playback % 160 == 0 && playback >= size + 320);
        for (unsigned i = size; i < playback; ++i)
            assert(orbit_sound_sample((orbit_sound_cue_t)cue, i) == 0);
        int peak = 0;
        FILE *file = NULL;
        if (argc == 2) { char path[512]; snprintf(path, sizeof(path), "%s/cue-%u.pcm", argv[1], cue); file = fopen(path, "wb"); assert(file); }
        for (unsigned i = 0; i < size; i++) {
            int16_t sample = orbit_sound_sample((orbit_sound_cue_t)cue, i);
            assert(abs(sample) <= 5400);
            if (abs(sample) > peak) peak = abs(sample);
            if (file) assert(fwrite(&sample, sizeof(sample), 1, file) == 1);
        }
        assert(peak >= 5250);
        if (file) fclose(file);
    }
    /* The forecast's inter-tone pause must be silent. */
    for (unsigned i = 90 * 16; i < 115 * 16; i++) assert(orbit_sound_sample(ORBIT_SOUND_FORECAST, i) == 0);
    double error = 0;
    unsigned length = orbit_sound_samples(ORBIT_SOUND_CLICK);
    for (unsigned i = 0; i < length; i++) {
        unsigned envelope = i < 256 ? i : length - 1 - i < 256 ? length - 1 - i : 256;
        double progress = envelope / 256.0;
        double fade = progress * progress * (3 - 2 * progress);
        double ideal = 5400.0 * fade * sin(6.283185307179586 * i * 1108 / 16000);
        double delta = orbit_sound_sample(ORBIT_SOUND_CLICK, i) - ideal;
        error += delta * delta;
    }
    assert(sqrt(error / length) < 18.0);
    assert(orbit_sound_playback_samples(ORBIT_SOUND_CLICK) == 1600);
    for (unsigned cue = 0; cue < ORBIT_SOUND_PREVIEW; ++cue)
        for (unsigned i = 0; i < orbit_sound_playback_samples((orbit_sound_cue_t)cue); ++i)
            assert(orbit_sound_output_volume((orbit_sound_cue_t)cue, i) == 70);
    for (unsigned i = 0; i < orbit_sound_samples(ORBIT_SOUND_CLICK); ++i)
        assert(orbit_sound_sample(ORBIT_SOUND_PREVIEW, i) == orbit_sound_sample(ORBIT_SOUND_CLICK, i));
    for (unsigned i = 0; i < orbit_sound_samples(ORBIT_SOUND_FORECAST); ++i)
        assert(orbit_sound_sample(ORBIT_SOUND_PREVIEW, 430 * 16 + i) == orbit_sound_sample(ORBIT_SOUND_FORECAST, i));
    for (unsigned i = 0; i < orbit_sound_samples(ORBIT_SOUND_COMPLETE); ++i)
        assert(orbit_sound_sample(ORBIT_SOUND_PREVIEW, 1005 * 16 + i) == orbit_sound_sample(ORBIT_SOUND_COMPLETE, i));
    assert(orbit_sound_output_volume(ORBIT_SOUND_PREVIEW, 0) == 70);
    assert(orbit_sound_output_volume(ORBIT_SOUND_PREVIEW, orbit_sound_playback_samples(ORBIT_SOUND_PREVIEW)) == 70);
    puts("Aligned playback blocks, silent tail, smooth edges, distinct click/forecast/completion, bounded volume, fade edges and silence PASS");
    return 0;
}
