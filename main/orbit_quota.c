#include "orbit_quota.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "mbedtls/gcm.h"
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"
#include "nvs.h"
#include <stdio.h>
#include <time.h>
static SemaphoreHandle_t lock;
static orbit_quota_t state;
static uint8_t key[32], pending_key[32], boot[16];
static int64_t pair_until;
static httpd_handle_t server;
static bool unhex(const char *s, size_t n, uint8_t *out) {
    for (size_t i = 0; i < n; ++i) {
        unsigned value = 0;
        for (unsigned j = 0; j < 2; ++j) {
            unsigned char c = s[i * 2 + j];
            if (c >= '0' && c <= '9') value = value * 16 + c - '0';
            else if (c >= 'a' && c <= 'f') value = value * 16 + c - 'a' + 10;
            else return false;
        }
        out[i] = (uint8_t)value;
    }
    return true;
}
static void expire_pair(void) {
    if (state.pairing && esp_timer_get_time() > pair_until) {
        state.pairing = state.offer = false; memset(pending_key, 0, sizeof(pending_key));
    }
}
void orbit_quota_read(orbit_quota_t *out) {
    xSemaphoreTake(lock, portMAX_DELAY); expire_pair();
    esp_netif_t *net = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip;
    state.ip[0] = 0;
    if (net && esp_netif_get_ip_info(net, &ip) == ESP_OK && ip.ip.addr)
        snprintf(state.ip, sizeof(state.ip), IPSTR, IP2STR(&ip.ip));
    *out = state; xSemaphoreGive(lock);
}
void orbit_quota_begin(void) {
    xSemaphoreTake(lock, portMAX_DELAY);
    if (!state.paired) { state.pairing = true; state.offer = false; pair_until = esp_timer_get_time() + 120000000; }
    xSemaphoreGive(lock);
}
void orbit_quota_cancel(void) {
    xSemaphoreTake(lock, portMAX_DELAY); state.pairing = state.offer = false;
    memset(pending_key, 0, sizeof(pending_key)); xSemaphoreGive(lock);
}
bool orbit_quota_confirm(void) {
    xSemaphoreTake(lock, portMAX_DELAY); expire_pair();
    bool success = false; nvs_handle_t nvs;
    if (state.pairing && state.offer && nvs_open("orbit_quota", NVS_READWRITE, &nvs) == ESP_OK) {
        if (nvs_set_blob(nvs, "key", pending_key, sizeof(pending_key)) == ESP_OK && nvs_commit(nvs) == ESP_OK) {
            memcpy(key, pending_key, sizeof(key)); state.paired = true; state.valid = false; state.sequence = 0;
            state.pairing = state.offer = false; esp_fill_random(boot, sizeof(boot)); success = true;
            uint8_t digest[32];
            if (mbedtls_sha256(key, sizeof(key), digest, 0) == 0) {
                char ack[86] = "ORBIT_PAIR_ACCEPTED ";
                for (unsigned i = 0; i < 32; ++i) snprintf(ack + 20 + i * 2, 3, "%02x", digest[i]);
                puts(ack);
            }
        }
        nvs_close(nvs);
    }
    state.offer = false;
    memset(pending_key, 0, sizeof(pending_key)); xSemaphoreGive(lock);
    return success;
}
void orbit_quota_revoke(void) {
    xSemaphoreTake(lock, portMAX_DELAY); nvs_handle_t nvs;
    if (nvs_open("orbit_quota", NVS_READWRITE, &nvs) == ESP_OK) {
        esp_err_t e = nvs_erase_key(nvs, "key");
        if ((e == ESP_OK || e == ESP_ERR_NVS_NOT_FOUND) && nvs_commit(nvs) == ESP_OK) {
            memset(&state, 0, sizeof(state)); memset(key, 0, sizeof(key)); memset(pending_key, 0, sizeof(pending_key));
            esp_fill_random(boot, sizeof(boot));
        }
        nvs_close(nvs);
    }
    xSemaphoreGive(lock);
}
static esp_err_t challenge(httpd_req_t *r) {
    char body[96], hex[33]; xSemaphoreTake(lock, portMAX_DELAY);
    for (unsigned i = 0; i < 16; ++i) snprintf(hex + i * 2, 3, "%02x", boot[i]);
    snprintf(body, sizeof(body), "{\"version\":1,\"paired\":%s,\"boot\":\"%s\"}", state.paired ? "true" : "false", hex);
    xSemaphoreGive(lock); httpd_resp_set_type(r, "application/json");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store"); return httpd_resp_sendstr(r, body);
}
static esp_err_t discover(httpd_req_t *r) {
    char query[48], nonce_hex[33], boot_hex[33], proof_hex[65];
    uint8_t nonce[16], message[48], proof[32];
    if (httpd_req_get_url_query_str(r, query, sizeof(query)) != ESP_OK ||
        httpd_query_key_value(query, "nonce", nonce_hex, sizeof(nonce_hex)) != ESP_OK ||
        strlen(nonce_hex) != sizeof(nonce_hex) - 1 || !unhex(nonce_hex, 16, nonce))
        return httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "Invalid nonce");

    xSemaphoreTake(lock, portMAX_DELAY);
    bool paired = state.paired;
    int error = -1;
    if (paired) {
        memcpy(message, "OrbitDiscover-v1", 16);
        memcpy(message + 16, nonce, sizeof(nonce));
        memcpy(message + 32, boot, sizeof(boot));
        const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        if (info) error = mbedtls_md_hmac(info, key, sizeof(key), message, sizeof(message), proof);
    }
    if (!paired || error != 0) {
        xSemaphoreGive(lock);
        memset(message, 0, sizeof(message));
        memset(proof, 0, sizeof(proof));
        memset(nonce, 0, sizeof(nonce));
        return httpd_resp_send_err(r, HTTPD_404_NOT_FOUND, "Unavailable");
    }
    for (unsigned i = 0; i < sizeof(boot); ++i)
        snprintf(boot_hex + i * 2, 3, "%02x", boot[i]);
    for (unsigned i = 0; i < sizeof(proof); ++i)
        snprintf(proof_hex + i * 2, 3, "%02x", proof[i]);
    xSemaphoreGive(lock);
    memset(message, 0, sizeof(message));
    memset(proof, 0, sizeof(proof));
    memset(nonce, 0, sizeof(nonce));
    char body[192];
    snprintf(body, sizeof(body), "{\"version\":1,\"nonce\":\"%s\",\"boot\":\"%s\",\"proof\":\"%s\"}",
             nonce_hex, boot_hex, proof_hex);
    httpd_resp_set_type(r, "application/json");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    return httpd_resp_sendstr(r, body);
}
static esp_err_t quota_post(httpd_req_t *r) {
    if (r->content_len != ORBIT_QUOTA_PACKET_BYTES) return httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "Invalid packet");
    uint8_t packet[ORBIT_QUOTA_PACKET_BYTES], plain[ORBIT_QUOTA_PAYLOAD_BYTES];
    size_t got = 0;
    while (got < sizeof(packet)) {
        int n = httpd_req_recv(r, (char *)packet + got, sizeof(packet) - got);
        if (n <= 0) return ESP_FAIL;
        got += (size_t)n;
    }
    xSemaphoreTake(lock, portMAX_DELAY);
    uint8_t aad[32] = {0}; memcpy(aad, "OrbitQuota-v1", 13); memcpy(aad + 16, boot, 16);
    mbedtls_gcm_context gcm; mbedtls_gcm_init(&gcm);
    int error = state.paired ? mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256) : -1;
    if (!error) error = mbedtls_gcm_auth_decrypt(&gcm, sizeof(plain), packet, 12, aad, sizeof(aad), packet + 56, 16, packet + 12, plain);
    bool success = !error && orbit_quota_decode(plain, sizeof(plain), time(NULL), state.sequence, &state);
    mbedtls_gcm_free(&gcm); memset(plain, 0, sizeof(plain)); xSemaphoreGive(lock);
    if (!success) return httpd_resp_send_err(r, HTTPD_403_FORBIDDEN, "Unauthorized or stale packet");
    return httpd_resp_sendstr(r, "OK");
}
static void serial_pair(void *arg) {
    (void)arg;
    char line[128] = {0}; size_t length = 0; bool overflow = false;
    for (;;) {
        int ch = getchar();
        if (ch == EOF) { clearerr(stdin); vTaskDelay(pdMS_TO_TICKS(20)); continue; }
        if (ch == '\r') continue;
        if (ch != '\n') {
            if (length < sizeof(line) - 1 && !overflow) line[length++] = (char)ch;
            else overflow = true;
            continue;
        }
        line[length] = 0;
        if (overflow) { memset(line, 0, sizeof(line)); length = 0; overflow = false; continue; }
        if (length == 75 && !memcmp(line, "ORBIT_PAIR ", 11)) {
            uint8_t proposal[32];
            if (unhex(line + 11, 32, proposal)) {
                xSemaphoreTake(lock, portMAX_DELAY); expire_pair();
                if (state.pairing && !state.paired) {
                    memcpy(pending_key, proposal, sizeof(proposal)); state.offer = true;
                    puts("ORBIT_PAIR_OFFER");
                }
                xSemaphoreGive(lock); memset(proposal, 0, sizeof(proposal));
            }
        } else if (!strcmp(line, "ORBIT_STATUS")) {
            orbit_quota_t q; orbit_quota_read(&q);
            printf("ORBIT_STATUS %s %s\n", q.paired ? "paired" : "unpaired", q.ip[0] ? q.ip : "offline");
        }
        memset(line, 0, sizeof(line)); length = 0;
    }
}
void orbit_quota_start(void) {
    lock = xSemaphoreCreateMutex(); configASSERT(lock); esp_fill_random(boot, sizeof(boot));
    nvs_handle_t nvs;
    if (nvs_open("orbit_quota", NVS_READONLY, &nvs) == ESP_OK) {
        size_t size = sizeof(key); state.paired = nvs_get_blob(nvs, "key", key, &size) == ESP_OK && size == sizeof(key); nvs_close(nvs);
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); config.server_port = ORBIT_QUOTA_PORT; config.ctrl_port = 32769;
    config.max_uri_handlers = 3; config.max_open_sockets = 3; config.stack_size = 4096;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t get = {.uri="/challenge",.method=HTTP_GET,.handler=challenge};
        httpd_uri_t find = {.uri="/discover",.method=HTTP_GET,.handler=discover};
        httpd_uri_t post = {.uri="/quota",.method=HTTP_POST,.handler=quota_post};
        if (httpd_register_uri_handler(server, &get) != ESP_OK ||
            httpd_register_uri_handler(server, &find) != ESP_OK ||
            httpd_register_uri_handler(server, &post) != ESP_OK) { httpd_stop(server); server = NULL; }
    }
    configASSERT(xTaskCreate(serial_pair, "quota_pair", 3072, NULL, 2, NULL) == pdPASS);
}
