#include "orbit_net.h"
#include "orbit_audio.h"
#include "orbit_notification.h"
#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs.h"
#include "orbit_access.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ONLINE BIT0
#define PROVISION BIT1
#define ASSOCIATED BIT2
static SemaphoreHandle_t mutex;
static EventGroupHandle_t events;
static QueueHandle_t credentials;
static orbit_network_t state;
static httpd_handle_t server;
static const char *TAG = "orbit";
typedef struct {
    char ssid[33], password[65];
} wifi_pair_t;
static wifi_pair_t saved;
static orbit_parser_t parser;
static void link_state(orbit_link_t value) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    state.link = value;
    xSemaphoreGive(mutex);
}
void orbit_net_read(orbit_network_t *out) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    *out = state;
    xSemaphoreGive(mutex);
}
void orbit_net_provision(void) { xEventGroupSetBits(events, PROVISION); }
void orbit_net_sound(bool enabled) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    state.sound = enabled;
    xSemaphoreGive(mutex);
    orbit_audio_enabled(enabled);
}
static bool request_notification(const orbit_snapshot_t *old, const orbit_snapshot_t *next,
                                 int64_t now, bool sound) {
    orbit_notification_t plan = orbit_notification_plan(old, next, now, sound);
    if (plan.play) orbit_audio_request(plan.cue);
    return plan.new_message;
}
#ifdef ORBIT_NOTIFICATION_REPLAY
/* Test firmware only: separate RAM history; never replace or persist real announcements. */
static void replay_notifications(void) {
    static unsigned stage;
    static int64_t deadline;
    static orbit_snapshot_t prior;
    orbit_network_t current;
    orbit_net_read(&current);
    int64_t tick = esp_timer_get_time() / 1000000;
    int64_t now = time(NULL);
    if (stage >= 6 || current.link != NET_ONLINE || !current.snapshot.received) return;
    if (!deadline) {
        if (!current.sound) return;
        prior = current.snapshot;
        deadline = tick + 20;
    }
    if (tick < deadline) return;
    static const char *labels[] = {"测试1 · 新预告", "测试2 · 重复静音", "测试3 · 完成消息",
        "测试4 · 重复静音", "测试5 · 关闭静音", "测试结束 · 真实消息"};
    orbit_snapshot_t next = prior;
    bool sound = current.sound;
    if (stage == 0 || stage == 4) {
        next.kind = ORBIT_FORECAST;
        snprintf(next.id, sizeof(next.id), "local-test-forecast-%u", stage);
    } else if (stage == 2) {
        next.kind = ORBIT_COMPLETED;
        snprintf(next.id, sizeof(next.id), "local-test-complete");
    }
    next.published = now; next.checked = now; next.received = now; next.healthy = true;
    if (stage == 4) sound = false;
    bool notify = stage < 5 && request_notification(&prior, &next, now, sound);
    ESP_LOGI("orbit_notify_test", "stage=%u new_message=%d sound=%d", stage + 1, notify, sound);
    prior = next;
    xSemaphoreTake(mutex, portMAX_DELAY);
    snprintf(state.notification_test, sizeof(state.notification_test), "%s", labels[stage]);
    xSemaphoreGive(mutex);
    stage++; deadline = tick + 8;
}
#endif
static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_CONNECTED) {
        xEventGroupSetBits(events, ASSOCIATED);
        ESP_LOGI(TAG, "Wi-Fi associated; awaiting DHCP");
    }
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        const wifi_event_sta_disconnected_t *failure = data;
        xEventGroupClearBits(events, ONLINE | ASSOCIATED);
        xSemaphoreTake(mutex, portMAX_DELAY);
        if (state.link == NET_CONNECTING)
            state.wifi_reason = failure ? failure->reason : 0;
        xSemaphoreGive(mutex);
        ESP_LOGW(TAG, "Wi-Fi disconnected; reason=%u", failure ? failure->reason : 0);
    }
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        wifi_ap_record_t ap;
        if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK)
            xEventGroupSetBits(events, ONLINE);
        ESP_LOGI(TAG, "Wi-Fi acquired an IP address");
    }
}
static const char FORM[] =
    "<!doctype html><html lang='zh-CN'><meta charset='utf-8'><meta name='viewport' "
    "content='width=device-width,initial-scale=1'><title>随身星盘配网</title><style>body{font:17px "
    "system-ui;max-width:420px;margin:40px "
    "auto;padding:24px;background:#101829;color:#eee}input,button{box-sizing:border-box;width:100%;"
    "padding:14px;margin:12px 0;border-radius:10px}small{color:#b8c5d9}</style><h1>连接家里的 "
    "Wi-Fi</h1><p>请选择 2.4 GHz 网络。密码仅发送给你手中的设备。</p><form method='post' "
    "action='/connect'><label>Wi-Fi 名称<input name='ssid' required maxlength='32' "
    "autocomplete='off'></label><label>Wi-Fi 密码<input name='password' type='password' "
    "maxlength='63' "
    "autocomplete='off'></label><button>连接并保存</button></"
    "form><small>"
    "验证成功后，设备会自动关闭配网热点。若连接失败，原有网络设置会保留。此页面仅配网，不登录 "
    "Codex。</small></html>";
static bool request_from_ap(httpd_req_t *r) {
    esp_netif_ip_info_t ap;
    struct sockaddr_storage peer = {0};
    socklen_t size = sizeof(peer);
    esp_netif_t *interface = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    return interface && getpeername(httpd_req_to_sockfd(r), (struct sockaddr *)&peer, &size) == 0 &&
           esp_netif_get_ip_info(interface, &ap) == ESP_OK &&
           orbit_peer_on_subnet(&peer, ap.ip.addr, ap.netmask.addr);
}
static esp_err_t require_hotspot(httpd_req_t *r) {
    httpd_resp_set_status(r, "403 Forbidden");
    httpd_resp_set_type(r, "text/plain; charset=utf-8");
    return httpd_resp_sendstr(r, "请连接设备屏幕显示的热点，再打开此页面。");
}
static esp_err_t get_form(httpd_req_t *r) {
    if (!request_from_ap(r))
        return require_hotspot(r);
    httpd_resp_set_type(r, "text/html; charset=utf-8");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    return httpd_resp_send(r, FORM, HTTPD_RESP_USE_STRLEN);
}
static esp_err_t post_form(httpd_req_t *r) {
    /* GET and POST share the same dual-stack-aware AP boundary. */
    if (!request_from_ap(r))
        return require_hotspot(r);
    char host[64], origin[80];
    if (httpd_req_get_hdr_value_str(r, "Host", host, sizeof(host)) != ESP_OK ||
        (strcmp(host, "192.168.4.1") && strcmp(host, "192.168.4.1:80")))
        return httpd_resp_send_err(r, HTTPD_403_FORBIDDEN, "Invalid host");
    if (httpd_req_get_hdr_value_len(r, "Origin") &&
        (httpd_req_get_hdr_value_str(r, "Origin", origin, sizeof(origin)) != ESP_OK ||
         strcmp(origin, "http://192.168.4.1")))
        return httpd_resp_send_err(r, HTTPD_403_FORBIDDEN, "Invalid origin");
    if (r->content_len <= 0 || r->content_len > 384)
        return httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "Form too large");
    char body[385];
    size_t at = 0;
    while (at < r->content_len) {
        int n = httpd_req_recv(r, body + at, r->content_len - at);
        if (n <= 0) {
            memset(body, 0, sizeof(body));
            return httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "Incomplete form");
        }
        at += (size_t)n;
    }
    body[at] = 0;
    wifi_pair_t pair = {0};
    bool valid = orbit_decode_form(body, pair.ssid, pair.password);
    memset(body, 0, sizeof(body));
    if (!valid) {
        memset(&pair, 0, sizeof(pair));
        return httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST,
                                   "Invalid Wi-Fi name or password length");
    }
    BaseType_t queued = xQueueSend(credentials, &pair, 0);
    memset(&pair, 0, sizeof(pair));
    if (queued != pdTRUE) {
        httpd_resp_set_status(r, "503 Service Unavailable");
        return httpd_resp_sendstr(r, "Connecting; please wait");
    }
    httpd_resp_set_type(r, "text/html; charset=utf-8");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    return httpd_resp_sendstr(
        r,
        "<meta charset='utf-8'><meta name='viewport' "
        "content='width=device-width,initial-scale=1'><h2>正在验证连接</"
        "h2><p>请查看设备屏幕。成功后热点会关闭；失败时请返回修改密码。</p><a href='/'>返回</a>");
}
static void stop_setup(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
    esp_wifi_set_mode(WIFI_MODE_STA);
    xSemaphoreTake(mutex, portMAX_DELAY);
    state.setup = false;
    memset(state.ap_password, 0, sizeof(state.ap_password));
    xSemaphoreGive(mutex);
}
static bool start_setup(void) {
    if (server)
        stop_setup();
    uint32_t nonce = esp_random(), secret = esp_random();
    wifi_config_t ap = {0};
    snprintf((char *)ap.ap.ssid, sizeof(ap.ap.ssid), "Starshore-%04lX",
             (unsigned long)(nonce & 65535));
    snprintf((char *)ap.ap.password, sizeof(ap.ap.password), "%08lX%04lX", (unsigned long)secret,
             (unsigned long)(esp_random() & 65535));
    ap.ap.ssid_len = strlen((char *)ap.ap.ssid);
    ap.ap.channel = 1;
    ap.ap.max_connection = 1;
    ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
    if (esp_wifi_set_mode(WIFI_MODE_APSTA) != ESP_OK ||
        esp_wifi_set_config(WIFI_IF_AP, &ap) != ESP_OK)
        return false;
    httpd_config_t c = HTTPD_DEFAULT_CONFIG();
    c.max_open_sockets = 3;
    c.lru_purge_enable = true;
    c.stack_size = 5120;
    c.recv_wait_timeout = 5;
    c.send_wait_timeout = 5;
    if (httpd_start(&server, &c) != ESP_OK) {
        esp_wifi_set_mode(WIFI_MODE_STA);
        return false;
    }
    const httpd_uri_t get = {.uri = "/", .method = HTTP_GET, .handler = get_form},
                      post = {.uri = "/connect", .method = HTTP_POST, .handler = post_form};
    if (httpd_register_uri_handler(server, &get) != ESP_OK ||
        httpd_register_uri_handler(server, &post) != ESP_OK) {
        stop_setup();
        return false;
    }
    xSemaphoreTake(mutex, portMAX_DELAY);
    state.setup = true;
    state.link = NET_SETUP;
    snprintf(state.ap_name, sizeof(state.ap_name), "%.23s", ap.ap.ssid);
    snprintf(state.ap_password, sizeof(state.ap_password), "%.15s", ap.ap.password);
    xSemaphoreGive(mutex);
    memset(&ap, 0, sizeof(ap));
    ESP_LOGI(TAG, "Temporary setup started; free heap %lu",
             (unsigned long)esp_get_free_heap_size());
    return true;
}
static bool configure(const wifi_pair_t *pair) {
    wifi_config_t config = {0};
    memcpy(config.sta.ssid, pair->ssid, strlen(pair->ssid));
    memcpy(config.sta.password, pair->password, strlen(pair->password));
    config.sta.pmf_cfg.capable = true;
    config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    config.sta.failure_retry_cnt = 2;
    config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    /* Reconfigure only STA. Stopping the whole radio also drops the phone's AP link. */
    esp_err_t e = esp_wifi_disconnect();
    if (e != ESP_OK && e != ESP_ERR_WIFI_NOT_CONNECT) {
        ESP_LOGE(TAG, "Wi-Fi disconnect before configure: %s", esp_err_to_name(e));
    }
    xEventGroupClearBits(events, ONLINE | ASSOCIATED);
    xSemaphoreTake(mutex, portMAX_DELAY);
    state.wifi_reason = 0;
    xSemaphoreGive(mutex);
    e = esp_wifi_set_config(WIFI_IF_STA, &config);
    memset(&config, 0, sizeof(config));
    if (e == ESP_OK)
        e = esp_wifi_connect();
    if (e != ESP_OK) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        state.wifi_reason = (unsigned)e;
        xSemaphoreGive(mutex);
        ESP_LOGE(TAG, "Wi-Fi configure/connect failed: %s", esp_err_to_name(e));
    } else
        ESP_LOGI(TAG, "Wi-Fi connection attempt started (bounded retries enabled)");
    return e == ESP_OK;
}
static esp_err_t http_event(esp_http_client_event_t *e) {
    if (e->event_id == HTTP_EVENT_ON_DATA && !orbit_parser_feed(&parser, e->data, e->data_len))
        return ESP_FAIL;
    return ESP_OK;
}
static bool fetch(orbit_snapshot_t *out) {
    orbit_parser_init(&parser);
    esp_http_client_config_t config = {.url = "https://www.starshoreai.com/quota-orbit/api/monitor",
                                       .crt_bundle_attach = esp_crt_bundle_attach,
                                       .event_handler = http_event,
                                       .timeout_ms = 20000,
                                       .buffer_size = 2048,
                                       .disable_auto_redirect = true};
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
        return false;
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_header(client, "Accept-Encoding", "identity");
    esp_err_t result = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    bool complete = esp_http_client_is_complete_data_received(client);
    esp_http_client_cleanup(client);
    return result == ESP_OK && status == 200 && complete &&
           orbit_parser_finish(&parser, time(NULL), out);
}
static void worker(void *arg) {
    (void)arg;
    nvs_handle_t nvs = 0;
    bool have_nvs = nvs_open("orbit", NVS_READWRITE, &nvs) == ESP_OK;
    if (have_nvs) {
        size_t n = sizeof(saved);
        if (nvs_get_blob(nvs, "wifi", &saved, &n) != ESP_OK || n != sizeof(saved))
            memset(&saved, 0, sizeof(saved));
        saved.ssid[32] = 0;
        saved.password[64] = 0;
        orbit_snapshot_t cache;
        n = sizeof(cache);
        if (nvs_get_blob(nvs, "snapshot2", &cache, &n) == ESP_OK && n == sizeof(cache)) {
            cache.id[31] = cache.revision[31] = 0;
            cache.url[159] = 0;
            xSemaphoreTake(mutex, portMAX_DELAY);
            state.snapshot = cache;
            xSemaphoreGive(mutex);
        }
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t wc = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wc));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t initial = {0};
    memcpy(initial.sta.ssid, saved.ssid, strlen(saved.ssid));
    memcpy(initial.sta.password, saved.password, strlen(saved.password));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &initial));
    memset(&initial, 0, sizeof(initial));
    ESP_ERROR_CHECK(esp_wifi_start());
    if (saved.ssid[0])
        esp_wifi_connect();
    esp_sntp_config_t ntp = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    ESP_ERROR_CHECK(esp_netif_sntp_init(&ntp));
    int64_t setup_until = 0, attempt_until = 0, next_fetch = 0, reconnect = 0, last_saved = 0;
    wifi_pair_t candidate = {0};
    bool attempting = false;
    if (!saved.ssid[0])
        xEventGroupSetBits(events, PROVISION);
    for (;;) {
        int64_t now = esp_timer_get_time() / 1000000;
        EventBits_t bits = xEventGroupGetBits(events);
        if (bits & PROVISION) {
            xEventGroupClearBits(events, PROVISION);
            if (start_setup())
                setup_until = now + 600;
            else
                link_state(NET_ERROR);
        }
        if (server && !attempting && xQueueReceive(credentials, &candidate, 0) == pdTRUE) {
            attempting = configure(&candidate);
            attempt_until = now + 45;
            setup_until = now + 600;
            link_state(attempting ? NET_CONNECTING : NET_BAD_WIFI);
        }
        bits = xEventGroupGetBits(events);
        if (attempting && (bits & ONLINE)) {
            if (have_nvs && nvs_set_blob(nvs, "wifi", &candidate, sizeof(candidate)) == ESP_OK &&
                nvs_commit(nvs) == ESP_OK) {
                saved = candidate;
                memset(&candidate, 0, sizeof(candidate));
                attempting = false;
                stop_setup();
                setup_until = 0;
                next_fetch = 0;
            } else {
                attempting = false;
                link_state(NET_ERROR);
            }
        } else if (attempting && now >= attempt_until) {
            attempting = false;
            ESP_LOGW(TAG, "Wi-Fi attempt expired; associated=%d", !!(bits & ASSOCIATED));
            xSemaphoreTake(mutex, portMAX_DELAY);
            if (bits & ASSOCIATED)
                state.wifi_reason = 10000; /* DHCP did not complete. */
            else if (!state.wifi_reason)
                state.wifi_reason = 10001; /* No driver reason. */
            state.link = NET_BAD_WIFI;
            xSemaphoreGive(mutex);
            esp_wifi_disconnect();
            memset(&candidate, 0, sizeof(candidate));
        }
        if (server && now >= setup_until) {
            stop_setup();
            attempting = false;
            memset(&candidate, 0, sizeof(candidate));
            if (saved.ssid[0])
                configure(&saved);
            link_state(NET_SETUP_EXPIRED);
        }
        if (!server) {
            if (!(bits & ONLINE)) {
                if (now >= reconnect && saved.ssid[0]) {
                    esp_wifi_connect();
                    reconnect = now + 15;
                }
                if (saved.ssid[0])
                    link_state(NET_OFFLINE);
            } else if (time(NULL) < 1700000000)
                link_state(NET_CLOCK);
            else if (now >= next_fetch) {
                link_state(NET_SYNCING);
                orbit_snapshot_t next;
                if (fetch(&next)) {
                    orbit_network_t old;
                    orbit_net_read(&old);
                    bool notify = request_notification(&old.snapshot, &next, time(NULL), old.sound);
                    xSemaphoreTake(mutex, portMAX_DELAY);
                    state.snapshot = next;
                    state.link = NET_ONLINE;
                    state.new_message = notify;
                    xSemaphoreGive(mutex);
                    /* Persist only a changed record or a half-hour checkpoint to limit flash wear.
                     */
                    if (have_nvs &&
                        (strcmp(old.snapshot.id, next.id) ||
                         strcmp(old.snapshot.revision, next.revision) ||
                         old.snapshot.kind != next.kind || next.received - last_saved >= 1800)) {
                        if (nvs_set_blob(nvs, "snapshot2", &next, sizeof(next)) == ESP_OK &&
                            nvs_commit(nvs) == ESP_OK)
                            last_saved = next.received;
                    }
                    next_fetch = now + 300;
                    ESP_LOGI(TAG, "Public sync OK; free heap %lu",
                             (unsigned long)esp_get_free_heap_size());
                } else {
                    link_state(NET_OFFLINE);
                    next_fetch = now + 60;
                }
            }
        }
#ifdef ORBIT_NOTIFICATION_REPLAY
        replay_notifications();
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void orbit_net_start(bool sound_enabled) {
    mutex = xSemaphoreCreateMutex();
    events = xEventGroupCreate();
    credentials = xQueueCreate(1, sizeof(wifi_pair_t));
    configASSERT(mutex && events && credentials);
    state.link = NET_CONNECTING;
    state.sound = sound_enabled;
    orbit_audio_start(sound_enabled);
    configASSERT(xTaskCreate(worker, "orbit_net", 7168, NULL, 4, NULL) == pdPASS);
}
