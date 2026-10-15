#!/usr/bin/env python3
"""Execute the real STA configuration function against a stateful radio fake."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'main/orbit_net.c').read_text()
start = source.index('static bool configure(')
end = source.index('static esp_err_t http_event', start)
function = source[start:end]
preamble = r'''
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_WIFI_NOT_CONNECT 1
#define WIFI_IF_STA 0
#define WIFI_ALL_CHANNEL_SCAN 1
#define WPA3_SAE_PWE_BOTH 2
#define ONLINE 1
#define ASSOCIATED 4
#define portMAX_DELAY 0
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
typedef struct {char ssid[33],password[65];} wifi_pair_t;
typedef struct {struct {unsigned char ssid[32],password[64];struct {bool capable;} pmf_cfg;int scan_method,failure_retry_cnt,sae_pwe_h2e;} sta;} wifi_config_t;
static int mutex,events,set_result,connect_result,connect_calls,stop_calls;
static bool ap_running=true;
static struct {unsigned wifi_reason;} state;
static wifi_config_t applied;
static void xSemaphoreTake(int x,int y){(void)x;(void)y;}
static void xSemaphoreGive(int x){(void)x;}
static void xEventGroupClearBits(int x,int y){(void)x;assert(y==(ONLINE|ASSOCIATED));}
static int esp_wifi_disconnect(void){assert(ap_running);return ESP_ERR_WIFI_NOT_CONNECT;}
static int esp_wifi_set_config(int which,const wifi_config_t *c){assert(which==WIFI_IF_STA);applied=*c;return set_result;}
static int esp_wifi_connect(void){assert(ap_running);connect_calls++;return connect_result;}
'''
tests = r'''
int main(void){
 wifi_pair_t pair={"sample-network","sample-password"};
 assert(configure(&pair));assert(ap_running && !stop_calls && connect_calls==1);
 assert(!strcmp((char*)applied.sta.ssid,pair.ssid));
 assert(!strcmp((char*)applied.sta.password,pair.password));
 assert(applied.sta.scan_method==WIFI_ALL_CHANNEL_SCAN && applied.sta.failure_retry_cnt==2);
 assert(applied.sta.sae_pwe_h2e==WPA3_SAE_PWE_BOTH);
 set_result=77;connect_calls=0;assert(!configure(&pair));assert(!connect_calls && state.wifi_reason==77);
 set_result=0;connect_result=88;assert(!configure(&pair));assert(state.wifi_reason==88 && ap_running);
 connect_result=0;assert(configure(&pair));assert(state.wifi_reason==0 && ap_running);
 puts("STA configure: preserved AP, bounded retries, error propagation PASS");
}
'''
with tempfile.TemporaryDirectory(prefix='orbit-radio-test-') as directory:
    c = Path(directory) / 'test.c'
    executable = Path(directory) / 'test'
    c.write_text(preamble + function + tests)
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(c), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
