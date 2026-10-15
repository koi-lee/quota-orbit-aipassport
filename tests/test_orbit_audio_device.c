#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef int esp_err_t;
#define ESP_OK 0
static bool initialized, sleeping;
static int stage, fail_stage, writes;
static esp_err_t bsp_audio_init(void) { stage = 1; if (fail_stage == stage) return -1; initialized = true; return 0; }
static esp_err_t bsp_audio_wake(void) { assert(stage == 1); stage = 2; if (fail_stage == stage) return -2; sleeping = false; return 0; }
static esp_err_t bsp_audio_set_format(uint32_t hz, uint8_t bits, uint8_t ch) {
    assert(stage == 2 && initialized && !sleeping);
    assert(hz == 16000 && bits == 16 && ch == 1);
    stage = 3; return fail_stage == stage ? -3 : 0;
}
static void bsp_audio_set_volume(uint8_t volume) { assert(stage == 3 && volume == 70); stage = 4; }
#include "../main/orbit_audio_device.h"
int main(void) {
    for (int i = 0; i < 30; ++i) {
        assert(orbit_audio_device_ready() == ESP_OK && stage == 4);
        assert(!sleeping); ++writes;
        sleeping = true;
    }
    assert(writes == 30);
    for (fail_stage = 1; fail_stage <= 3; ++fail_stage) {
        assert(orbit_audio_device_ready() == -fail_stage);
        assert(stage == fail_stage);
    }
    fail_stage = 0;
    assert(orbit_audio_device_ready() == ESP_OK);
    puts("Audio first start, 30 sleep/wake cycles and staged failure recovery PASS");
}
