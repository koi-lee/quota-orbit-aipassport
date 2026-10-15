#include "orbit_wish.h"
#include "orbit_wish_model.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "mbedtls/sha256.h"
#include "nvs.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>

static SemaphoreHandle_t mutex;
static QueueHandle_t clicks;
static orbit_wish_view_t view;
static bool desired, overflow;
static uint32_t generation;
static char response[2048];
static size_t response_size;

void orbit_wish_read(orbit_wish_view_t *out) {
    xSemaphoreTake(mutex, portMAX_DELAY); *out = view; xSemaphoreGive(mutex);
}
void orbit_wish_set_enabled(bool enabled) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (view.ready && desired != enabled) {
        desired = enabled; ++generation; overflow = false;
        view.enabled = enabled; view.status = enabled ? WISH_PENDING : WISH_OFF;
        if (!enabled) view.pending = 0;
    }
    xSemaphoreGive(mutex);
}
void orbit_wish_click(void) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (view.ready && desired && xQueueSend(clicks, &generation, 0) != pdTRUE) overflow = true;
    xSemaphoreGive(mutex);
}
static void publish(const orbit_wish_store_t *s, uint32_t epoch, orbit_wish_status_t status) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (generation == epoch) {
        view.enabled = s->enabled; view.pending = orbit_wish_pending(s);
        view.status = overflow && s->enabled ? WISH_FULL : status;
    }
    xSemaphoreGive(mutex);
}
static bool persist(nvs_handle_t nvs, const orbit_wish_store_t *s) {
    return nvs_set_blob(nvs, "queue1", s, sizeof(*s)) == ESP_OK && nvs_commit(nvs) == ESP_OK;
}
static void event_id(const orbit_wish_store_t *s, uint64_t sequence, char id[37]) {
    uint8_t input[40], digest[32]; memcpy(input, s->seed, 32);
    for (unsigned i = 0; i < 8; ++i) input[32 + i] = (uint8_t)(sequence >> (56 - i * 8));
    if (mbedtls_sha256(input, sizeof(input), digest, 0)) { id[0] = 0; return; }
    digest[6] = (digest[6] & 15) | 0x40; digest[8] = (digest[8] & 63) | 0x80;
    unsigned pos = 0;
    for (unsigned i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) id[pos++] = '-';
        snprintf(id + pos, 3, "%02x", digest[i]); pos += 2;
    }
}
static esp_err_t receive(esp_http_client_event_t *event) {
    if (event->event_id == HTTP_EVENT_ON_DATA) {
        if (event->data_len < 0 || (size_t)event->data_len >= sizeof(response) - response_size) return ESP_FAIL;
        memcpy(response + response_size, event->data, event->data_len);
        response_size += event->data_len; response[response_size] = 0;
    }
    return ESP_OK;
}
static bool send_batch(const orbit_wish_store_t *s, unsigned count) {
    char ids[ORBIT_WISH_BATCH][37], body[1400];
    size_t used = (size_t)snprintf(body, sizeof(body), "{\"events\":[");
    for (unsigned i = 0; i < count; ++i) {
        event_id(s, s->acknowledged + i, ids[i]);
        if (!ids[i][0]) return false;
        used += (size_t)snprintf(body + used, sizeof(body) - used, "%s\"%s\"", i ? "," : "", ids[i]);
    }
    used += (size_t)snprintf(body + used, sizeof(body) - used, "]}");
    response_size = 0; response[0] = 0;
    esp_http_client_config_t cfg = {.url = CONFIG_ORBIT_WISH_URL, .timeout_ms = 6000,
        .crt_bundle_attach = esp_crt_bundle_attach, .event_handler = receive,
        .disable_auto_redirect = true, .buffer_size = 512, .buffer_size_tx = 512};
    esp_http_client_handle_t http = esp_http_client_init(&cfg);
    if (!http) return false;
    esp_http_client_set_method(http, HTTP_METHOD_POST);
    esp_http_client_set_header(http, "Content-Type", "application/json");
    esp_http_client_set_post_field(http, body, (int)used);
    esp_err_t result = esp_http_client_perform(http);
    bool complete = esp_http_client_is_complete_data_received(http);
    int status = esp_http_client_get_status_code(http); esp_http_client_cleanup(http);
    if (result != ESP_OK || status != 200 || !complete) return false;
    const char *end = NULL;
    cJSON *json = cJSON_ParseWithOpts(response, &end, true);
    cJSON *accepted = cJSON_GetObjectItemCaseSensitive(json, "accepted");
    cJSON *total = cJSON_GetObjectItemCaseSensitive(json, "total");
    bool valid = cJSON_IsNumber(total) && total->valuedouble >= 0 &&
        total->valuedouble <= 9007199254740991.0 &&
        total->valuedouble == (double)(uint64_t)total->valuedouble &&
        cJSON_IsArray(accepted) && cJSON_GetArraySize(accepted) == (int)count;
    for (unsigned i = 0; valid && i < count; ++i) {
        unsigned matches = 0;
        cJSON *entry;
        cJSON_ArrayForEach(entry, accepted)
            if (cJSON_IsString(entry) && !strcmp(entry->valuestring, ids[i])) ++matches;
        if (matches != 1) valid = false;
    }
    cJSON_Delete(json); return valid;
}
static bool online(void) {
    esp_netif_t *net = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"); esp_netif_ip_info_t ip;
    return net && esp_netif_is_netif_up(net) && esp_netif_get_ip_info(net, &ip) == ESP_OK && ip.ip.addr;
}
static void worker(void *arg) {
    (void)arg;
    nvs_handle_t nvs;
    orbit_wish_store_t state; orbit_wish_disable(&state);
    if (nvs_open("orbit_wish", NVS_READWRITE, &nvs) != ESP_OK) {
        xSemaphoreTake(mutex, portMAX_DELAY); view.status = WISH_STORAGE_ERROR;
        xSemaphoreGive(mutex); vTaskDelete(NULL); return;
    }
    size_t size = sizeof(state);
    esp_err_t loaded = nvs_get_blob(nvs, "queue1", &state, &size);
    if (loaded != ESP_ERR_NVS_NOT_FOUND && (loaded != ESP_OK || size != sizeof(state) || !orbit_wish_store_valid(&state))) {
        nvs_close(nvs); xSemaphoreTake(mutex, portMAX_DELAY); view.status = WISH_STORAGE_ERROR;
        xSemaphoreGive(mutex); vTaskDelete(NULL); return;
    }
    /* Only a missing record receives defaults; saved opt-outs remain off. */
    if (loaded == ESP_ERR_NVS_NOT_FOUND) {
        uint8_t seed[32]; esp_fill_random(seed, sizeof(seed));
        if (!orbit_wish_enable(&state, seed) || !persist(nvs, &state)) {
            nvs_close(nvs); xSemaphoreTake(mutex, portMAX_DELAY); view.status = WISH_STORAGE_ERROR;
            xSemaphoreGive(mutex); vTaskDelete(NULL); return;
        }
    }
    xSemaphoreTake(mutex, portMAX_DELAY);
    desired = state.enabled; view.ready = true; view.enabled = desired;
    xSemaphoreGive(mutex);
    uint32_t epoch = 0; bool dirty = false, storage_ok = true;
    int64_t last_save = 0, next_send = 0; unsigned failures = 0;
    for (;;) {
        bool requested; uint32_t wanted;
        xSemaphoreTake(mutex, portMAX_DELAY); requested = desired; wanted = generation;
        if (wanted != epoch) {
            epoch = wanted; failures = 0; next_send = 0;
            if (requested) { uint8_t seed[32]; esp_fill_random(seed, sizeof(seed)); orbit_wish_enable(&state, seed); }
            else orbit_wish_disable(&state);
            dirty = true;
        }
        uint32_t click;
        unsigned drained = 0;
        while (drained++ < 64 && xQueueReceive(clicks, &click, 0) == pdTRUE) {
            if (click == epoch && state.enabled) {
                if (orbit_wish_add(&state)) dirty = true;
                else overflow = true;
            }
        }
        xSemaphoreGive(mutex);
        int64_t now = esp_timer_get_time() / 1000;
        if (dirty && now - last_save >= 500) {
            storage_ok = persist(nvs, &state); last_save = now;
            if (storage_ok) dirty = false;
        }
        orbit_wish_status_t status = !storage_ok ? WISH_STORAGE_ERROR : !state.enabled ? WISH_OFF :
            !orbit_wish_pending(&state) ? WISH_READY : !CONFIG_ORBIT_WISH_URL[0] ? WISH_UNAVAILABLE :
            !online() ? WISH_OFFLINE : WISH_PENDING;
        xSemaphoreTake(mutex, portMAX_DELAY);
        bool allowed = desired && generation == epoch;
        xSemaphoreGive(mutex);
        if (allowed && state.enabled && !dirty && storage_ok && orbit_wish_pending(&state) &&
            !strncmp(CONFIG_ORBIT_WISH_URL, "https://", 8) && online() && now >= next_send) {
            unsigned count = orbit_wish_pending(&state);
            if (count > ORBIT_WISH_BATCH) count = ORBIT_WISH_BATCH;
            if (send_batch(&state, count)) {
                /* Persisting the new prefix may fail; retries use the same event IDs. */
                orbit_wish_store_t acknowledged = state;
                orbit_wish_ack(&acknowledged, state.acknowledged, count);
                storage_ok = persist(nvs, &acknowledged);
                if (storage_ok) { state = acknowledged; failures = 0; status = WISH_SYNCED; }
                else { status = WISH_STORAGE_ERROR; dirty = true; }
                next_send = esp_timer_get_time() / 1000 + 1000;
            } else {
                if (failures < 5) ++failures;
                next_send = esp_timer_get_time() / 1000 + (1000LL << failures); status = WISH_RETRY;
            }
        }
        publish(&state, epoch, status);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
void orbit_wish_start(void) {
    mutex = xSemaphoreCreateMutex(); clicks = xQueueCreate(64, sizeof(uint32_t));
    configASSERT(mutex && clicks);
    configASSERT(xTaskCreate(worker, "orbit_wish", 8192, NULL, 3, NULL) == pdPASS);
}
