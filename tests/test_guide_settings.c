#include "guide_settings.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static bool found,fail_get,fail_set,fail_commit;
static uint8_t durable,pending;
static unsigned sets,commits;
esp_err_t nvs_get_u8(nvs_handle_t h,const char *key,uint8_t *value) {
    assert(h==7&&!strcmp(key,"volume"));if(fail_get||!found)return 2;*value=durable;return 0;
}
esp_err_t nvs_set_u8(nvs_handle_t h,const char *key,uint8_t value) {
    assert(h==7&&!strcmp(key,"volume"));sets++;if(fail_set)return 3;pending=value;return 0;
}
esp_err_t nvs_commit(nvs_handle_t h) {
    assert(h==7);commits++;if(fail_commit)return 4;durable=pending;found=true;return 0;
}
int main(void) {
    assert(guide_settings_load_volume(7,false)==95&&guide_settings_load_volume(7,true)==95);
    found=true;durable=255;assert(guide_settings_load_volume(7,true)==95);
    durable=0;assert(guide_settings_load_volume(7,true)==0);
    durable=100;assert(guide_settings_load_volume(7,true)==100);
    fail_get=true;assert(guide_settings_load_volume(7,true)==95);fail_get=false;
    assert(guide_settings_save_volume(7,false,85)!=0&&sets==0);
    assert(guide_settings_save_volume(7,true,255)!=0&&sets==0);
    fail_set=true;assert(guide_settings_save_volume(7,true,85)==3&&commits==0&&durable==100);fail_set=false;
    fail_commit=true;assert(guide_settings_save_volume(7,true,85)==4&&durable==100);fail_commit=false;
    assert(guide_settings_load_volume(7,true)==100);
    assert(guide_settings_save_volume(7,true,90)==0&&guide_settings_load_volume(7,true)==90);
    assert(guide_settings_save_volume(7,true,0)==0&&guide_settings_load_volume(7,true)==0);
    puts("Guide volume settings missing/invalid/mute/bounds/write/commit failure and reload PASS");
}
