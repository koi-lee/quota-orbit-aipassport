#include "orbit_quota_model.h"
#include <assert.h>
#include <stdio.h>
static void be(uint8_t *out, uint64_t value, unsigned n) {
    while (n) { out[--n]=(uint8_t)value; value >>= 8; }
}
int main(void) {
    uint8_t data[44]={1}; be(data+1,1800000000,8);be(data+9,1,8);data[17]=1;
    be(data+18,10080,4);data[22]=42;be(data+23,1800000100,8);
    orbit_quota_t q={.paired=true};
    assert(orbit_quota_decode(data,44,1800000000,0,&q));
    assert(q.valid && q.windows[0].remaining==42 && q.windows[0].minutes==10080);
    assert(q.windows[1].minutes==0); assert(orbit_quota_fresh(&q,1800000119));
    const orbit_quota_window_t *slots[2]; orbit_quota_slots(&q, slots);
    assert(!slots[0] && slots[1] == &q.windows[0]);
    assert(!orbit_quota_fresh(&q,1800000121));
    assert(orbit_quota_fresh(&q,1799999990));
    assert(!orbit_quota_fresh(&q,1799999980));
    assert(!orbit_quota_decode(data,44,1800000000,1,&q));
    assert(!orbit_quota_decode(data,43,1800000000,0,&q));
    assert(!orbit_quota_decode(data,44,1799999984,0,&q));
    assert(!orbit_quota_decode(data,44,1800000121,0,&q));
    data[22]=101; assert(!orbit_quota_decode(data,44,1800000000,0,&q));data[22]=42;
    data[17]=0; assert(!orbit_quota_decode(data,44,1800000000,0,&q));
    assert(q.windows[0].remaining==42);
    char summary[120], age[120];
    orbit_quota_summary(&q, 1800000000, summary, sizeof(summary), age, sizeof(age));
    assert(!strcmp(summary, "周额度剩余 42%"));
    assert(!strcmp(age, "刚刚同步 · 来自 Mac"));
    assert(!strstr(summary, "未提供") && !strstr(summary, "短"));
    orbit_quota_summary(&q, 1800000180, summary, sizeof(summary), age, sizeof(age));
    assert(!strcmp(age, "旧数据 · 3分钟前同步"));
    q.windows[1] = (orbit_quota_window_t){300, 68, 1800000001};
    orbit_quota_summary(&q, 1800000000, summary, sizeof(summary), age, sizeof(age));
    assert(!strcmp(summary, "5小时额度 68% / 周额度 42%"));
    q.windows[1].minutes = 0;
    q.windows[0].minutes = 15; q.windows[1] = (orbit_quota_window_t){60, 80, 1800000001};
    orbit_quota_slots(&q, slots); assert(slots[0] == &q.windows[0] && slots[1] == &q.windows[1]);
    puts("Quota exact framing, absent window, bounds, clock freshness and replay rejection PASS");
}
