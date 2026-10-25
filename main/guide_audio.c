#include "guide_audio.h"
#include "guide_adpcm.h"
#include "guide_voice_assets.h"
#include "bsp_audio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static TaskHandle_t worker;
static portMUX_TYPE lock=portMUX_INITIALIZER_UNLOCKED;
static guide_audio_status_t status={.spot=GUIDE_NONE};
static uint8_t desired_volume=95;
void guide_audio_set_volume(uint8_t volume) {
    taskENTER_CRITICAL(&lock); desired_volume=volume>100?100:volume; taskEXIT_CRITICAL(&lock);
}
static uint8_t requested_volume(void) {
    taskENTER_CRITICAL(&lock); uint8_t value=desired_volume; taskEXIT_CRITICAL(&lock); return value;
}
guide_audio_status_t guide_audio_status(void) {
    taskENTER_CRITICAL(&lock); guide_audio_status_t copy=status; taskEXIT_CRITICAL(&lock); return copy;
}
static bool current(uint32_t generation) { return guide_audio_status().generation==generation; }
static void publish(uint32_t generation,guide_voice_state_t state) {
    taskENTER_CRITICAL(&lock);
    if(status.generation==generation) { status.state=state; if(state==GUIDE_VOICE_OFF) status.spot=GUIDE_NONE; }
    taskEXIT_CRITICAL(&lock);
}
static void request(uint8_t spot) {
    taskENTER_CRITICAL(&lock);
    status.generation++; status.spot=spot;
    status.state=worker?(spot==GUIDE_NONE?GUIDE_VOICE_STOPPING:GUIDE_VOICE_STARTING):GUIDE_VOICE_ERROR;
    taskEXIT_CRITICAL(&lock);
    if(worker) xTaskNotifyGive(worker);
}
void guide_audio_play(uint8_t spot) { if(spot<GUIDE_SPOTS) { if(!worker) (void)guide_audio_start(); request(spot); } }
void guide_audio_stop(void) {
    guide_audio_status_t s=guide_audio_status();
    if(s.state!=GUIDE_VOICE_OFF && s.state!=GUIDE_VOICE_STOPPING) request(GUIDE_NONE);
}
static esp_err_t ready(uint8_t volume) {
    esp_err_t e=bsp_audio_init();
    if(e==ESP_OK) e=bsp_audio_wake();
    if(e==ESP_OK) e=bsp_audio_set_format(16000,16,1);
    if(e==ESP_OK) e=bsp_audio_set_volume_checked(volume);
    return e;
}
static void audio_worker(void *unused) {
    (void)unused;
    for(;;) {
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        guide_audio_status_t s=guide_audio_status();
        if(s.spot==GUIDE_NONE) {
            esp_err_t sleeping=bsp_audio_sleep();
            publish(s.generation,sleeping==ESP_OK?GUIDE_VOICE_OFF:GUIDE_VOICE_ERROR); continue;
        }
        const guide_voice_clip_t *clip=&guide_voice_clips[s.spot];
        uint8_t applied=requested_volume();
        esp_err_t error=ready(applied);
        if(error!=ESP_OK) {
            bsp_audio_sleep(); publish(s.generation,GUIDE_VOICE_ERROR);
            ESP_LOGW("guide_audio","Prepare failed: %s; UI remains available",esp_err_to_name(error)); continue;
        }
        if(!current(s.generation)) { bsp_audio_sleep(); continue; }
        guide_adpcm_t decoder={0}; int16_t pcm[160]; int16_t tail=0;
        int64_t last_feed=0; uint32_t longest_feed=0; uint64_t energy=0; unsigned sent=0,peak=0;
        if(current(s.generation)) publish(s.generation,GUIDE_VOICE_PLAYING);
        ESP_LOGI("guide_audio","Start spot=%u samples=%u",s.spot,(unsigned)clip->samples);
        while(current(s.generation)) {
            uint8_t requested=requested_volume();
            if(requested!=applied) { error=bsp_audio_set_volume_checked(requested); if(error!=ESP_OK) break; applied=requested; }
            size_t count=guide_adpcm_decode(&decoder,clip->data,clip->bytes,clip->samples,pcm,160);
            if(!count) break;
            /* Pad the last DMA block with silence; never read past a clip. */
            tail=pcm[count-1];
            for(size_t i=count;i<160;i++) pcm[i]=0;
            int64_t feed=esp_timer_get_time();
            if(last_feed && feed-last_feed>longest_feed) longest_feed=(uint32_t)(feed-last_feed);
            last_feed=feed;
            error=bsp_audio_write(pcm,sizeof(pcm));
            if(error!=ESP_OK) break;
            for(size_t i=0;i<count;i++) { int sample=pcm[i]; unsigned level=(unsigned)(sample<0?-sample:sample); if(level>peak) peak=level; energy+=(uint64_t)(sample*sample); }
            sent+=(unsigned)count;
        }
        if(!current(s.generation) && error==ESP_OK) {
            for(unsigned i=0;i<160;i++) pcm[i]=(int16_t)((int)tail*(159-(int)i)/160);
            error=bsp_audio_write(pcm,sizeof(pcm));
        }
        esp_err_t sleeping=bsp_audio_sleep();
        if(error==ESP_OK) error=sleeping;
        if(error!=ESP_OK) {
            publish(s.generation,GUIDE_VOICE_ERROR); ESP_LOGW("guide_audio","Playback failed: %s",esp_err_to_name(error));
        } else {
            publish(s.generation,GUIDE_VOICE_OFF); ESP_LOGI("guide_audio","Stopped spot=%u samples_sent=%u peak=%u mean_square=%llu max_feed_us=%lu stack_free=%u",s.spot,sent,peak,(unsigned long long)(sent?energy/sent:0),(unsigned long)longest_feed,(unsigned)uxTaskGetStackHighWaterMark(NULL));
        }
    }
}
bool guide_audio_start(void) {
    if(worker) return true;
    if(xTaskCreate(audio_worker,"guide_audio",4096,NULL,5,&worker)!=pdPASS) { worker=NULL; status.state=GUIDE_VOICE_ERROR; return false; }
    return true;
}
