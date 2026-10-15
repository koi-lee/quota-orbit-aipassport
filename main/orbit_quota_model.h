#include "orbit_language.h"
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#define ORBIT_QUOTA_PAYLOAD_BYTES 44
#define ORBIT_QUOTA_PACKET_BYTES 72
#define ORBIT_QUOTA_PORT 8899
#define ORBIT_QUOTA_FRESH_SECONDS 120
typedef struct { uint32_t minutes; uint8_t remaining; int64_t reset; } orbit_quota_window_t;
typedef struct {
    orbit_quota_window_t windows[2];
    uint64_t sequence;
    int64_t updated;
    bool valid, paired, pairing, offer;
    char ip[16];
} orbit_quota_t;
static inline uint64_t orbit_quota_be(const uint8_t *p, size_t n) {
    uint64_t value = 0; while (n--) value = value * 256 + *p++; return value;
}
static inline bool orbit_quota_decode(const uint8_t *p, size_t length, int64_t now, uint64_t last, orbit_quota_t *out) {
    if (length != ORBIT_QUOTA_PAYLOAD_BYTES || p[0] != 1 || p[17] < 1 || p[17] > 2 || now < 1700000000) return false;
    uint64_t stamp = orbit_quota_be(p + 1, 8), seq = orbit_quota_be(p + 9, 8);
    if (stamp > (uint64_t)now + 15 || stamp + ORBIT_QUOTA_FRESH_SECONDS < (uint64_t)now || seq <= last) return false;
    orbit_quota_t next = *out;
    memset(next.windows, 0, sizeof(next.windows));
    for (unsigned i = 0; i < p[17]; ++i) {
        const uint8_t *w = p + 18 + i * 13;
        uint64_t reset = orbit_quota_be(w + 5, 8);
        uint32_t minutes = (uint32_t)orbit_quota_be(w, 4);
        if (!minutes || minutes > 525600 || w[4] > 100 || reset > 4102444800ULL) return false;
        next.windows[i] = (orbit_quota_window_t){minutes, w[4], (int64_t)reset};
    }
    next.valid = true; next.updated = (int64_t)stamp; next.sequence = seq;
    *out = next; return true;
}
static inline bool orbit_quota_fresh(const orbit_quota_t *q, int64_t now) {
    return q->valid && q->updated <= now + 15 && now - q->updated <= ORBIT_QUOTA_FRESH_SECONDS;
}
static inline void orbit_quota_slots(const orbit_quota_t *q, const orbit_quota_window_t *slots[2]) {
    slots[0] = slots[1] = NULL;
    if (!q->valid) return;
    for (unsigned i = 0; i < 2; ++i) {
        const orbit_quota_window_t *w = &q->windows[i];
        if (!w->minutes) continue;
        if (w->minutes == 10080 && !slots[1]) slots[1] = w;
        else if (!slots[0]) slots[0] = w;
        else if (!slots[1]) slots[1] = w;
    }
}

static inline void orbit_quota_window_title(const orbit_quota_window_t *w, char *out, size_t cap) {
    if (w->minutes == 10080) snprintf(out, cap, "%s", orbit_tr("周额度"));
    else if (w->minutes == 300) snprintf(out, cap, "%s", orbit_tr("5小时额度"));
    else snprintf(out, cap, orbit_tr("%lu分钟额度"), (unsigned long)w->minutes);
}
static inline void orbit_quota_summary(const orbit_quota_t *q, int64_t now,
                                      char *summary, size_t summary_cap, char *age, size_t age_cap) {
    const orbit_quota_window_t *slots[2]; orbit_quota_slots(q, slots);
    if (!q->paired) {
        snprintf(summary, summary_cap, "%s", orbit_tr("连接 Mac 查看个人额度"));
        snprintf(age, age_cap, "%s", orbit_tr("设置 → 连接 Mac"));
        return;
    }
    if (!q->valid) {
        snprintf(summary, summary_cap, "%s", orbit_tr("等待 Mac 同步个人额度"));
        snprintf(age, age_cap, "%s", orbit_tr("请在 Mac 查看同步状态"));
        return;
    }
    if (!slots[0] && !slots[1]) {
        snprintf(summary, summary_cap, "%s", orbit_tr("个人额度暂未提供"));
        snprintf(age, age_cap, "%s", orbit_tr("等待 Mac 更新"));
        return;
    }
    char titles[2][40];
    for (unsigned i = 0; i < 2; ++i)
        if (slots[i]) orbit_quota_window_title(slots[i], titles[i], sizeof(titles[i]));
    if (slots[0] && slots[1]) {
        snprintf(summary, summary_cap, "%s %u%% / %s %u%%", titles[0], slots[0]->remaining,
                 titles[1], slots[1]->remaining);
    } else {
        unsigned i = slots[0] ? 0 : 1;
        snprintf(summary, summary_cap, orbit_tr("%s剩余 %u%%"), titles[i], slots[i]->remaining);
    }
    int64_t elapsed = now > q->updated ? now - q->updated : 0;
    if (!orbit_quota_fresh(q, now))
        snprintf(age, age_cap, orbit_tr("旧数据 · %lld分钟前同步"), (long long)(elapsed / 60));
    else if (elapsed < 60) snprintf(age, age_cap, "%s", orbit_tr("刚刚同步 · 来自 Mac"));
    else snprintf(age, age_cap, "%s", orbit_tr("1分钟前同步 · 来自 Mac"));
}
