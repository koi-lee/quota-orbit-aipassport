#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "guide_ui.h"
#include "guide_audio.h"
#include "guide_settings.h"
#include "key_guard.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "orbit_input.h"
#include "orbit_power.h"

static QueueHandle_t input;
static key_guard_t guard;
static orbit_input_t edges;
static orbit_power_t power={.brightness=ORBIT_POWER_BRIGHT};
static portMUX_TYPE power_lock=portMUX_INITIALIZER_UNLOCKED;
static volatile bool s_input_ready;
static uint32_t dropped;
static void on_key(bsp_btn_t key,bsp_btn_ev_t event,void *user) {
    (void)user;
    if(key_guard_consume(&guard,event==BSP_BTN_RELEASE,esp_timer_get_time())) { edges=(orbit_input_t){0}; return; }
    if(!s_input_ready) return;
    orbit_input_event_t edge=event==BSP_BTN_PRESS?ORBIT_INPUT_PRESS:event==BSP_BTN_RELEASE?ORBIT_INPUT_RELEASE:event==BSP_BTN_LONG?ORBIT_INPUT_LONG:ORBIT_INPUT_OTHER;
    taskENTER_CRITICAL(&power_lock);
    bool wake=orbit_power_edge(&power,(unsigned)key,edge,esp_timer_get_time());
    taskEXIT_CRITICAL(&power_lock);
    if(wake) { edges=(orbit_input_t){0}; return; }
    int action;
    if(orbit_input_edge(&edges,(unsigned)key,edge,&action) && xQueueSend(input,&action,0)!=pdTRUE) {
        taskENTER_CRITICAL(&power_lock); dropped++; taskEXIT_CRITICAL(&power_lock);
    }
}
static bool write_record(void *context,unsigned slot,const uint8_t *bytes,size_t len) {
    nvs_handle_t h=*(nvs_handle_t *)context;
    esp_err_t err=nvs_set_blob(h,slot?"slot1":"slot0",bytes,len);
    if(err==ESP_OK) err=nvs_commit(h);
    if(err!=ESP_OK) ESP_LOGW("guide","Record save failed: %s",esp_err_to_name(err));
    return err==ESP_OK;
}
void app_main(void) {
    /* Shared NVS belongs to other apps too. Never erase it to recover this app. */
    nvs_handle_t h=0;
    bool storage=nvs_flash_init()==ESP_OK;
    if(storage) storage=nvs_open("pocket_guide",NVS_READWRITE,&h)==ESP_OK;
    uint8_t a[GUIDE_RECORD_SIZE],b[GUIDE_RECORD_SIZE]; size_t na=sizeof(a),nb=sizeof(b);
    bool prior_record=false;
    if(storage) {
        esp_err_t ea=nvs_get_blob(h,"slot0",a,&na), eb=nvs_get_blob(h,"slot1",b,&nb);
        prior_record=ea!=ESP_ERR_NVS_NOT_FOUND || eb!=ESP_ERR_NVS_NOT_FOUND;
        if(ea!=ESP_OK) na=0;
        if(eb!=ESP_OK) nb=0;
    } else { na=0; nb=0; }
    guide_record_t record; int slot=guide_record_select(&record,a,na,b,nb);
    if(slot<0 && prior_record) storage=false;
    guide_model_t model; guide_init(&model,record); model.storage_ok=storage;
    model.volume=guide_settings_load_volume(h,storage);
    ESP_ERROR_CHECK(bsp_i2c_init());
    ESP_ERROR_CHECK(bsp_display_init());
    if(!bsp_lvgl_init()) return;
    bsp_display_backlight(ORBIT_POWER_BRIGHT);
    bool battery_ok=bsp_battery_init()==ESP_OK;
    int battery=battery_ok?bsp_battery_soc():-1;
    bool audio_ok=guide_audio_start();
    guide_audio_set_volume(model.volume);
    if(!audio_ok) ESP_LOGW("guide","Audio task unavailable; browsing remains available");
    input=xQueueCreate(16,sizeof(int)); configASSERT(input);
    key_guard_arm(&guard,esp_timer_get_time(),1000000);
    ESP_ERROR_CHECK(bsp_button_init(on_key,NULL));
    key_guard_sync_released(&guard,bsp_button_read_mv()>=BSP_BTN_MV_RELEASED_MIN);
    vTaskPrioritySet(NULL,4);
    power.active_us=esp_timer_get_time(); s_input_ready=true;
    bool dirty=true; uint8_t brightness=ORBIT_POWER_BRIGHT;
    int64_t last_battery=esp_timer_get_time(),notice_until=0;
    bool volume_save_pending=false;
    ESP_LOGI("guide","Pocket Guide v0.5.0; 22 offline spots; storage=%s; volume=%u; free_heap=%lu",storage?"ready":"unavailable",model.volume,(unsigned long)esp_get_free_heap_size());
    for(;;) {
        int action;
        if(xQueueReceive(input,&action,pdMS_TO_TICKS(50))==pdTRUE) {
            guide_record_t before=model.record;
            uint8_t old_volume=model.volume;
            if(guide_key(&model,action)) {
                if(storage && guide_record_save(&model.record,&slot,write_record,&h)) model.storage_ok=true;
                else { model.record=before; model.storage_ok=false; model.notice="未能保存，原记录已保留"; }
            }
            if(model.command==GUIDE_COMMAND_PLAY) guide_audio_play(model.nav.spot);
            if(model.command==GUIDE_COMMAND_STOP) guide_audio_stop();
            if(model.volume!=old_volume) guide_audio_set_volume(model.volume);
            if(model.save_volume) volume_save_pending=true;
            notice_until=model.notice?esp_timer_get_time()+2000000:0; dirty=true;
        }
        guide_audio_status_t audio=guide_audio_status();
        if(model.voice!=audio.state) { model.voice=audio.state; dirty=true; }
        if(volume_save_pending && (audio.state==GUIDE_VOICE_OFF || audio.state==GUIDE_VOICE_ERROR)) {
            esp_err_t error=guide_settings_save_volume(h,storage,model.volume);
            model.notice=error==ESP_OK?"音量已保存":"音量未保存，本次可用";
            ESP_LOGI("guide","Volume preference %u save=%s",model.volume,esp_err_to_name(error));
            volume_save_pending=false; notice_until=esp_timer_get_time()+2000000; dirty=true;
        }
        int64_t now=esp_timer_get_time();
        if(notice_until && now>=notice_until) { model.notice=NULL; notice_until=0; dirty=true; }
        taskENTER_CRITICAL(&power_lock);
        uint8_t target=orbit_power_tick(&power,now,false);
        taskEXIT_CRITICAL(&power_lock);
        if(target!=brightness) { bsp_display_backlight(target); brightness=target; ESP_LOGI("guide","Backlight %u%%",target); }
        if(now-last_battery>=60000000) { battery=battery_ok?bsp_battery_soc():-1; last_battery=now; dirty=true; }
        if(dirty && bsp_lvgl_lock(100)) { guide_ui_draw(&model,battery); bsp_lvgl_unlock(); dirty=false; }
        taskENTER_CRITICAL(&power_lock);
        uint32_t overflow=dropped; dropped=0;
        taskEXIT_CRITICAL(&power_lock);
        if(overflow) ESP_LOGW("guide","Input queue overflow: %lu",(unsigned long)overflow);
    }
}
