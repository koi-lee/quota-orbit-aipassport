#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "guide_audio.h"
#include "guide_voice_assets.h"
#include "bsp_audio.h"
#include "freertos/task.h"
static jmp_buf idle;
static unsigned pending,writes,sleeps,prepares,mode,fail_stage,stage;
static bool fail_create;
static uint8_t last_volume;
static unsigned volume_calls;
static uint8_t codes[160];
const guide_voice_clip_t guide_voice_clips[GUIDE_SPOTS]={{codes,160,320},{codes,160,320},{codes,160,320}};
const char *esp_err_to_name(esp_err_t error) { (void)error; return "test failure"; }
int xTaskCreate(void (*fn)(void *),const char *name,unsigned stack,void *arg,unsigned priority,TaskHandle_t *task) {
    (void)fn;(void)name;(void)arg;assert(stack==4096 && priority==5);
    if(fail_create) return 0;
    *task=(void *)1; return 1;
}
void xTaskNotifyGive(TaskHandle_t task) { assert(task);pending++; }
unsigned ulTaskNotifyTake(int clear,uint32_t timeout) { assert(clear && timeout==UINT32_MAX); if(!pending) longjmp(idle,1);pending=0;return 1; }
esp_err_t bsp_audio_init(void) { prepares++;stage=1;return fail_stage==1?-1:0; }
esp_err_t bsp_audio_wake(void) { assert(stage==1);stage=2;if(mode==1) guide_audio_stop();return fail_stage==2?-2:0; }
esp_err_t bsp_audio_set_format(uint32_t hz,uint8_t bits,uint8_t ch) { assert(stage==2&&hz==16000&&bits==16&&ch==1);stage=3;return fail_stage==3?-3:0; }
esp_err_t bsp_audio_set_volume_checked(uint8_t volume) { assert(stage==3&&volume<=100);last_volume=volume;volume_calls++;if(mode==5&&volume_calls==1)guide_audio_set_volume(75);if(mode==6&&volume_calls==2)return -7;return fail_stage==6?-6:0; }
esp_err_t bsp_audio_write(const void *pcm,size_t bytes) {
    assert(pcm&&bytes==320);writes++;
    if(mode==4 && writes==1) guide_audio_set_volume(100);
    if(mode==6 && writes==1) guide_audio_set_volume(90);
    if(mode==2 && writes==1) guide_audio_stop();
    if(mode==3 && writes==1) { guide_audio_play(1);guide_audio_play(2); }
    return fail_stage==4?-4:0;
}
esp_err_t bsp_audio_sleep(void) { sleeps++;return fail_stage==5?-5:0; }
#include "../main/guide_audio.c"
static void run(void) { if(!setjmp(idle)) audio_worker(NULL); }
static void reset(void) { pending=writes=sleeps=prepares=mode=fail_stage=stage=volume_calls=0;guide_audio_set_volume(95);status=(guide_audio_status_t){.spot=GUIDE_NONE}; }
int main(void) {
    fail_create=true;assert(!guide_audio_start());guide_audio_play(0);assert(status.state==GUIDE_VOICE_ERROR);
    fail_create=false;assert(guide_audio_start());reset();guide_audio_play(0);run();assert(writes==2&&sleeps==1&&status.state==GUIDE_VOICE_OFF);
    reset();mode=1;guide_audio_play(0);run();assert(!writes&&sleeps==2&&status.state==GUIDE_VOICE_OFF);
    reset();mode=2;guide_audio_play(0);run();assert(writes==2&&sleeps==2&&status.state==GUIDE_VOICE_OFF);
    reset();mode=3;guide_audio_play(0);run();assert(prepares==2&&sleeps==2&&writes==4&&status.state==GUIDE_VOICE_OFF);
    reset();guide_audio_play(0);guide_audio_play(1);guide_audio_play(2);run();assert(prepares==1&&writes==2); // no stale clips
    for(unsigned failure=1;failure<=6;failure++) {
        reset();fail_stage=failure;guide_audio_play(0);run();assert(status.state==GUIDE_VOICE_ERROR&&sleeps==1);
        fail_stage=0;guide_audio_play(2);run();assert(status.state==GUIDE_VOICE_OFF);
    }
    reset();fail_stage=5;guide_audio_play(0);run();guide_audio_stop();run();assert(status.state==GUIDE_VOICE_ERROR);
    fail_stage=0;guide_audio_stop();run();assert(status.state==GUIDE_VOICE_OFF);
    reset();guide_audio_play(22);assert(!pending&&status.state==GUIDE_VOICE_OFF);
    reset();mode=4;guide_audio_play(0);run();assert(volume_calls==2&&last_volume==100&&writes==2);
    reset();mode=5;guide_audio_play(0);run();assert(volume_calls==2&&last_volume==75);
    reset();guide_audio_set_volume(255);guide_audio_play(0);run();assert(last_volume==100);
    reset();mode=6;guide_audio_play(0);run();assert(writes==1&&volume_calls==2&&status.state==GUIDE_VOICE_ERROR);
    mode=0;guide_audio_play(0);run();assert(status.state==GUIDE_VOICE_OFF&&last_volume==90);
    reset();guide_audio_set_volume(35);assert(prepares==0&&pending==0);
    puts("Guide audio lazy start, cancellation, newest request, idle sleep and failure retry PASS");
}
