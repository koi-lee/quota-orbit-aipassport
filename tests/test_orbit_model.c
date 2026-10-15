#include "orbit_model.h"
#include "orbit_notification.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HEAD                                                                                       \
    "{\"ignored\":[1,true,null,{\"state\":\"completed\"}],\"publicInsights\":{\"schema_version\":" \
    "1,\"source_healthy\":true,\"source_checked_at\":\"2026-10-13T09:19:34Z\",\"events\":["
#define TAIL "]}}"
#define EVENT                                                                                      \
    "{\"event_id\":\"1\",\"type\":\"reset\",\"state\":\"completed\",\"published_at\":\"2026-10-"   \
    "13T09:06:10Z\",\"source_urls\":[\"https://x.com/thsottiaux/status/123\"]}"
static orbit_parser_t parser;
static const int64_t now = 1791883200;
static bool parse(const char *text, size_t chunk, orbit_snapshot_t *out) {
    orbit_parser_init(&parser);
    size_t len = strlen(text);
    for (size_t i = 0; i < len; i += chunk)
        if (!orbit_parser_feed(&parser, text + i, len - i < chunk ? len - i : chunk))
            return false;
    return orbit_parser_finish(&parser, now, out);
}
static void timing_tests(void) {
    orbit_snapshot_t s;
    char json[2048], lines[3][64];
    const char *base = "{\"event_id\":\"time\",\"type\":\"reset\",\"state\":\"%s\","
                       "\"published_at\":\"2026-10-13T09:06:10Z\","
                       "\"source_urls\":[\"https://x.com/test/status/1\"],%s}";
    struct {
        const char *state, *fields, *expected_line;
    } cases[] = {
        {"completed", "\"effective_at\":null", "完成  未单独公布"},
        {"completed", "\"effective_at\":\"2026-10-13T09:00:00Z\"", "完成  10/13 17:00"},
        {"completed", "\"effective_at\":\"2026-10-14T09:00:00Z\"", "完成  未单独公布"},
        {"scheduled", "\"time_kind\":\"expected\",\"expected_at\":\"2026-10-14T01:00:00Z\"",
         "预计  10/14 09:00"},
        {"scheduled", "\"time_kind\":\"deadline\",\"window_end\":\"2026-10-13T10:00:00Z\"",
         "预计最晚  10/13 18:00"},
        {"scheduled", "\"time_kind\":\"start\",\"window_start\":\"2026-10-13T10:00:00Z\"",
         "预计开始  10/13 18:00"},
        {"announced", "\"time_kind\":null,\"expected_at\":null", "预计  尚未明确时间"},
        {"announced", "\"time_kind\":\"unknown\",\"expected_at\":\"2026-10-13T10:00:00Z\"",
         "预计  尚未明确时间"},
        {"scheduled", "\"time_kind\":\"expected\",\"expected_at\":\"invalid\"",
         "预计  尚未明确时间"},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char event[1024];
        snprintf(event, sizeof(event), base, cases[i].state, cases[i].fields);
        snprintf(json, sizeof(json), "%s%s%s", HEAD, event, TAIL);
        for (size_t chunk = 1; chunk <= 137; chunk += 17) {
            assert(parse(json, chunk, &s));
            orbit_timing_lines(&s, now, lines);
            assert(!strcmp(lines[0], "发布  10/13 17:06"));
            assert(!strcmp(lines[1], cases[i].expected_line));
        }
    }
    assert(parse(HEAD EVENT TAIL, 1, &s));
    orbit_timing_lines(&s, now, lines);
    assert(!s.effective && !strcmp(lines[1], "完成  未单独公布"));
    assert(!strcmp(lines[2], "下次  暂无新预告"));
    s.kind = ORBIT_FORECAST;
    s.time_kind = ORBIT_TIME_EXPECTED;
    s.expected = now - 60;
    orbit_timing_lines(&s, now, lines);
    assert(strstr(lines[2], "尚未收到完成公告"));
    s.time_kind = ORBIT_TIME_WINDOW;
    s.window_start = now - 3600;
    s.window_end = now - 60;
    orbit_timing_lines(&s, now, lines);
    assert(strstr(lines[1], "预计开始") && strstr(lines[2], "原预计结束"));
    s.received = 0;
    orbit_timing_lines(&s, now, lines);
    assert(strstr(lines[0], "等待首次同步"));
    /* A newer completion must not inherit the older forecast's target time. */
    assert(parse(
        HEAD
        "{\"event_id\":\"old\",\"type\":\"reset\",\"state\":\"scheduled\","
        "\"published_at\":\"2026-10-13T08:00:00Z\",\"expected_at\":\"2026-10-13T10:00:00Z\","
        "\"time_kind\":\"expected\",\"source_urls\":[\"https://x.com/test/status/2\"]}," EVENT TAIL,
        3, &s));
    assert(s.kind == ORBIT_COMPLETED && !s.expected && !s.effective);
    assert(parse(
        HEAD "{\"event_id\":\"window\",\"type\":\"reset\",\"state\":\"scheduled\","
             "\"published_at\":\"2026-10-13T08:00:00Z\",\"time_kind\":\"window\","
             "\"window_start\":\"2026-10-13T11:00:00Z\",\"window_end\":\"2026-10-13T10:00:00Z\","
             "\"source_urls\":[\"https://x.com/test/status/3\"]}" TAIL,
        1, &s));
    assert(!s.window_start && !s.window_end);
    orbit_timing_lines(&s, now, lines);
    assert(strstr(lines[1], "尚未明确时间"));
}
int main(int argc, char **argv) {
    timing_tests();
    orbit_snapshot_t baseline = {.received = now - 60, .published = now - 60, .kind = ORBIT_COMPLETED};
    strcpy(baseline.id, "baseline");
    orbit_snapshot_t forecast = {.received = now, .checked = now, .published = now, .healthy = true, .kind = ORBIT_FORECAST};
    strcpy(forecast.id, "new-forecast");
    orbit_notification_t plan = orbit_notification_plan(&baseline, &forecast, now, true);
    assert(plan.new_message && plan.play && plan.cue == ORBIT_SOUND_FORECAST);
    assert(!orbit_notification_plan(&forecast, &forecast, now, true).play);
    orbit_snapshot_t completed = forecast; completed.kind = ORBIT_COMPLETED;
    plan = orbit_notification_plan(&forecast, &completed, now, true);
    assert(plan.new_message && plan.play && plan.cue == ORBIT_SOUND_COMPLETE);
    assert(!orbit_notification_plan(&completed, &completed, now, true).play);
    plan = orbit_notification_plan(&baseline, &forecast, now, false);
    assert(plan.new_message && !plan.play);
    orbit_snapshot_t empty = {0};
    assert(!orbit_notification_plan(&empty, &forecast, now, true).play);
    forecast.published = now - 1800;
    assert(!orbit_notification_plan(&baseline, &forecast, now, true).play);
    forecast.published = now + 1;
    assert(!orbit_notification_plan(&baseline, &forecast, now, true).play);
    orbit_snapshot_t s, old = {0};
    for (size_t i = 1; i < 500; i++) {
        assert(parse(HEAD EVENT TAIL, i, &s));
        assert(s.kind == ORBIT_COMPLETED);
        assert(!strcmp(s.id, "1"));
        assert(!orbit_is_stale(&s, now));
    }
    assert(!orbit_should_notify(&old, &s, now));
    old = s;
    assert(!orbit_should_notify(&old, &s, now));
    strcpy(old.id, "0");
    assert(orbit_should_notify(&old, &s, now));
    s.healthy = false;
    assert(!orbit_should_notify(&old, &s, now));
    s.healthy = true;
    assert(orbit_is_stale(&s, now + 1801));
    assert(parse(HEAD TAIL, 1, &s));
    assert(s.kind == ORBIT_EMPTY);
    assert(!parse(HEAD EVENT "," TAIL, 7, &s));
    assert(!parse(HEAD EVENT "]}", 7, &s));
    assert(!parse(HEAD EVENT TAIL "{}", 7, &s));
    assert(!parse("{\"publicInsights\":{\"events\":[]}}", 1, &s));
    assert(!parse("{\"x\":01}", 1, &s));
    assert(!parse("{\"x\":tru}", 1, &s));
    assert(!parse("{\"x\":\"\\q\"}", 1, &s));
    assert(orbit_iso_time("2026-10-13T09:20:00Z") == now);
    assert(!orbit_iso_time("2026-02-30T09:20:00Z"));
    assert(!orbit_iso_time("2026-10-13T09:20:00+08:00"));

    char changed[2048];
    const char *original = HEAD EVENT TAIL;
    struct {
        const char *from, *to;
        orbit_kind_t kind;
    } cases[] = {{"completed", "scheduled", ORBIT_FORECAST},
                 {"completed", "announced", ORBIT_FORECAST},
                 {"completed", "pending", ORBIT_EMPTY},
                 {"reset", "credit", ORBIT_EMPTY},
                 {"https://x.com/", "http://x.com/", ORBIT_EMPTY},
                 {"https://x.com/", "https://x.com.evil/", ORBIT_EMPTY}};
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        /* Select the event field, not the unrelated earlier object. */
        const char *at = strstr(strstr(original, "events"), cases[i].from);
        assert(at);
        size_t prefix = (size_t)(at - original);
        memcpy(changed, original, prefix);
        snprintf(changed + prefix, sizeof(changed) - prefix, "%s%s", cases[i].to,
                 at + strlen(cases[i].from));
        assert(parse(changed, 3, &s));
        assert(s.kind == cases[i].kind);
    }
    assert(parse(HEAD EVENT TAIL, 17, &s));
    old = s;
    strcpy(s.id, "new");
    s.published = now - 1801;
    assert(!orbit_should_notify(&old, &s, now));
    s.published = now + 301;
    assert(!orbit_should_notify(&old, &s, now));
    /* Deterministic malformed byte streams under ASan/UBSan exercise bounds. */
    unsigned seed = 42;
    for (int trial = 0; trial < 10000; trial++) {
        char bytes[256];
        for (size_t i = 0; i < sizeof(bytes); i++) {
            seed = seed * 1664525u + 1013904223u;
            bytes[i] = (char)(seed >> 24);
        }
        orbit_parser_init(&parser);
        orbit_parser_feed(&parser, bytes, sizeof(bytes));
    }
    char name[33], password[65];
    assert(orbit_decode_form("ssid=home+wifi&password=abc%26defgh", name, password));
    assert(!strcmp(name, "home wifi"));
    assert(!strcmp(password, "abc&defgh"));
    assert(orbit_decode_form("password=&ssid=open", name, password));
    assert(!orbit_decode_form("ssid=test&password=short", name, password));
    assert(!orbit_decode_form("ssid=test%00other&password=", name, password));
    assert(!orbit_decode_form("ssid=test&ssid=other&password=", name, password));
    assert(!orbit_decode_form("ssid=%ZZ&password=", name, password));
    if (argc > 1) {
        FILE *f = fopen(argv[1], "rb");
        assert(f);
        orbit_parser_init(&parser);
        char buf[137];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)))
            assert(orbit_parser_feed(&parser, buf, n));
        fclose(f);
        assert(orbit_parser_finish(&parser, now, &s));
        assert(s.kind == ORBIT_COMPLETED);
        assert(!strcmp(s.id, "2109873307722121591"));
        printf("Live fixture: %zu bytes streamed, parser %zu bytes\n", parser.total,
               sizeof(parser));
    }
    puts("Orbit model: PASS");
    return 0;
}
