#include "lvgl.h"
#include "orbit_ui.h"
#include "orbit_book_mask.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
LV_FONT_DECLARE(orbit_font_16);
LV_FONT_DECLARE(orbit_font_14);
LV_FONT_DECLARE(orbit_font_20);
LV_FONT_DECLARE(orbit_font_12);
LV_FONT_DECLARE(orbit_font_24);
LV_FONT_DECLARE(orbit_font_tc_12);
LV_FONT_DECLARE(orbit_font_tc_14);
LV_FONT_DECLARE(orbit_font_tc_16);
LV_FONT_DECLARE(orbit_font_tc_20);
LV_FONT_DECLARE(orbit_font_tc_24);
static int render_battery = 78;
static int64_t render_now = 1791883200;
static unsigned long flushed_pixels;
static unsigned char pixels[240 * 320 * 3];
static unsigned char draw[240 * 40 * 2];
static void flush(lv_display_t *d, const lv_area_t *a, uint8_t *map) {
    flushed_pixels += (unsigned long)(a->x2 - a->x1 + 1) * (a->y2 - a->y1 + 1);
    for (int y = a->y1; y <= a->y2; y++)
        for (int x = a->x1; x <= a->x2; x++) {
            unsigned value = (unsigned)map[0] | ((unsigned)map[1] << 8);
            map += 2;
            if (x < 0 || x >= 240 || y < 0 || y >= 320)
                continue;
            unsigned pos = (unsigned)(y * 240 + x) * 3;
            pixels[pos] = ((value >> 11) & 31) * 255 / 31;
            pixels[pos + 1] = ((value >> 5) & 63) * 255 / 63;
            pixels[pos + 2] = (value & 31) * 255 / 31;
        }
    lv_display_flush_ready(d);
}
static void check_labels(lv_obj_t *obj) {
    if (lv_obj_check_type(obj, &lv_label_class)) {
        const lv_font_t *font = lv_obj_get_style_text_font(obj, 0);
        if (lv_obj_get_height(obj) > font->line_height + 2)
            fprintf(stderr, "Wrapped label: %s\n", lv_label_get_text(obj));
        assert(lv_obj_get_height(obj) <= font->line_height + 2);
        lv_area_t area;
        lv_obj_get_coords(obj, &area);
        assert(area.x1 >= 0 && area.y1 >= 0 && area.x2 < 240 && area.y2 < 320);
    }
    for (uint32_t i = 0; i < lv_obj_get_child_count(obj); i++)
        check_labels(lv_obj_get_child(obj, i));
}
static void save(const char *dir, const char *name, orbit_ui_t *ui, orbit_network_t *net) {
    orbit_ui_draw(ui, net, render_battery, render_now);
    lv_refr_now(NULL);
    check_labels(lv_screen_active());
    if (ui->language != ORBIT_EN) {
        lv_obj_t *brand_zh = lv_obj_get_child(lv_screen_active(), 0);
        lv_obj_t *brand_en = lv_obj_get_child(lv_screen_active(), 1);
        int zh_center = 2 * lv_obj_get_y(brand_zh) + lv_obj_get_height(brand_zh);
        int en_center = 2 * lv_obj_get_y(brand_en) + lv_obj_get_height(brand_en);
        assert(abs(zh_center - en_center) <= 1);
        assert(lv_obj_get_x(brand_en) + lv_obj_get_width(brand_en) <= 180);
    }
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.ppm", dir, name);
    FILE *f = fopen(path, "wb");
    assert(f);
    fprintf(f, "P6\n240 320\n255\n");
    assert(fwrite(pixels, 1, sizeof(pixels), f) == sizeof(pixels));
    fclose(f);
}
static void motion_frame(const char *dir, const char *name, unsigned frame) {
    lv_tick_inc(40);
    lv_timer_handler();
    lv_refr_now(NULL);
    check_labels(lv_screen_active());
    static unsigned char partial[sizeof(pixels)];
    memcpy(partial, pixels, sizeof(pixels));
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(NULL);
    assert(memcmp(partial, pixels, sizeof(pixels)) == 0);
    char path[512];
    snprintf(path, sizeof(path), "%s/%s-%02u.ppm", dir, name, frame);
    FILE *f = fopen(path, "wb");
    assert(f);
    fprintf(f, "P6\n240 320\n255\n");
    assert(fwrite(pixels, 1, sizeof(pixels), f) == sizeof(pixels));
    fclose(f);
}
static void font_check(const char *path) {
    FILE *f = fopen(path, "rb");
    assert(f);
    int b;
    unsigned count = 0;
    while ((b = fgetc(f)) != EOF) {
        unsigned cp = (unsigned)b;
        if (cp < 32)
            continue;
        if (cp >= 0xC0) {
            unsigned extra = cp < 0xE0 ? 1 : cp < 0xF0 ? 2 : 3;
            cp &= (1u << (6 - extra)) - 1;
            while (extra--) {
                b = fgetc(f);
                assert(b != EOF && (b & 0xC0) == 0x80);
                cp = (cp << 6) | ((unsigned)b & 63);
            }
        }
        lv_font_glyph_dsc_t d;
        const lv_font_t *fonts[] = {&orbit_font_12,    &orbit_font_14,    &orbit_font_16,
                                    &orbit_font_20,    &orbit_font_24,    &orbit_font_tc_12,
                                    &orbit_font_tc_14, &orbit_font_tc_16, &orbit_font_tc_20,
                                    &orbit_font_tc_24};
        for (unsigned i = 0; i < 10; i++)
            assert(lv_font_get_glyph_dsc(fonts[i], &d, cp, 0) && !d.is_placeholder);
        count++;
    }
    fclose(f);
    printf("Font coverage: %u code points PASS\n", count);
}
int main(int argc, char **argv) {
    assert(argc >= 3 && argc <= 5);
    lv_init();
    font_check(argv[2]);
    lv_display_t *d = lv_display_create(240, 320);
    assert(d);
    lv_display_set_color_format(d, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(d, flush);
    lv_display_set_buffers(d, draw, NULL, sizeof(draw), LV_DISPLAY_RENDER_MODE_PARTIAL);
    orbit_ui_t ui = {0};
    const char *event_path = NULL;
    for (int i = 3; i < argc; i++) {
        if (!strncmp(argv[i], "--language=", 11)) {
            const char *name = argv[i] + 11;
            if (!strcmp(name, "zh-TW")) ui.language = ORBIT_ZH_HANT;
            else if (!strcmp(name, "en")) ui.language = ORBIT_EN;
            else assert(!strcmp(name, "zh-CN"));
        } else if (strcmp(argv[i], "--layout-only")) {
            assert(!event_path);
            event_path = argv[i];
        }
    }
    orbit_language_set(ui.language);
    orbit_network_t n = {.link = NET_ONLINE,
                         .snapshot = {.kind = ORBIT_COMPLETED,
                                      .published = 1791882370,
                                      .received = 1791883200,
                                      .checked = 1791883174,
                                      .healthy = true}};
    bool layout_only = false;
    if (event_path) {
        FILE *f = fopen(event_path, "rb");
        assert(f);
        static orbit_parser_t parser;
        orbit_parser_init(&parser);
        char data[137];
        size_t size;
        while ((size = fread(data, 1, sizeof(data), f)))
            assert(orbit_parser_feed(&parser, data, size));
        fclose(f);
        render_now = time(NULL);
        assert(orbit_parser_finish(&parser, render_now, &n.snapshot));
        save(argv[1], "live-home", &ui, &n);
        ui.page = 1;
        save(argv[1], "live-letter", &ui, &n);
        puts("Captured live API parse and layout PASS");
        return 0;
    }
    for (int i = 3; i < argc; i++)
        if (!strcmp(argv[i], "--layout-only")) layout_only = true;
    if (layout_only) {
        strcpy(n.snapshot.url, "https://example.invalid/source");
        save(argv[1], "home", &ui, &n);
        ui.page = 1;
        save(argv[1], "letter", &ui, &n);
        ui.page = 2;
        ui.quota.paired = true;
        ui.quota.valid = true;
        ui.quota.updated = render_now;
        ui.quota.windows[0] = (orbit_quota_window_t){10080, 55, render_now + 3600};
        save(argv[1], "character", &ui, &n);
        ui.settings = true;
        ui.settings_row = 3;
        save(argv[1], "settings", &ui, &n);
        ui.confirm_action = 1;
        save(argv[1], "confirm-wifi", &ui, &n);
        ui.confirm_action = 2;
        ui.quota.paired = true;
        save(argv[1], "confirm-unpair", &ui, &n);
        ui.confirm_action = 0;
        ui.settings = false;
        ui.page = 0;
        n.setup = true;
        n.link = NET_SETUP;
        strcpy(n.ap_name, "Passport-Demo");
        strcpy(n.ap_password, "DEMO12345678");
        save(argv[1], "setup", &ui, &n);
        const unsigned wifi_errors[] = {10000, 10001, 201, 202, 210, 999};
        for (unsigned i = 0; i < sizeof(wifi_errors) / sizeof(wifi_errors[0]); i++) {
            n.link = NET_BAD_WIFI;
            n.wifi_reason = wifi_errors[i];
            char name[32];
            snprintf(name, sizeof(name), "wifi-error-%u", wifi_errors[i]);
            save(argv[1], name, &ui, &n);
        }
        n.setup = false;
        n.wifi_reason = 0;
        n.link = NET_OFFLINE;
        save(argv[1], "offline", &ui, &n);
        n.link = NET_ONLINE;
        n.snapshot.kind = ORBIT_FORECAST;
        n.snapshot.time_kind = ORBIT_TIME_DEADLINE;
        n.snapshot.published = render_now - 10800;
        n.snapshot.window_end = render_now - 3600;
        save(argv[1], "overdue-forecast", &ui, &n);
        puts("Three-language representative layout PASS");
        return 0;
    }
    strcpy(n.snapshot.url, "https://x.com/thsottiaux/status/2109873307722121591");
    save(argv[1], "home", &ui, &n);
    const char *test_labels[] = {"测试1 · 新预告", "测试2 · 重复静音", "测试3 · 完成消息",
        "测试4 · 重复静音", "测试5 · 关闭静音", "测试结束 · 真实消息"};
    for (unsigned stage = 0; stage < 6; ++stage) {
        snprintf(n.notification_test, sizeof(n.notification_test), "%s", test_labels[stage]);
        char name[32]; snprintf(name, sizeof(name), "notify-test-%u", stage + 1);
        save(argv[1], name, &ui, &n);
    }
    n.notification_test[0] = 0;
    const int batteries[] = {0, 20, 50, 100, -1};
    const char *battery_names[] = {"battery-empty", "battery-low", "battery-half", "battery-full", "battery-unknown"};
    for (unsigned i = 0; i < 5; i++) {
        render_battery = batteries[i];
        save(argv[1], battery_names[i], &ui, &n);
    }
    render_battery = 78;
    assert(!orbit_ui_key(&ui, 0) && ui.page == 1);
    save(argv[1], "letter", &ui, &n);
    orbit_ui_key(&ui, 0);
    assert(ui.source);
    save(argv[1], "source", &ui, &n);
    n.snapshot.url[0] = 0;
    save(argv[1], "source-without-original", &ui, &n);
    strcpy(n.snapshot.url, "https://x.com/thsottiaux/status/2109873307722121591");
    orbit_ui_key(&ui, 2);
    assert(!ui.source && !ui.settings);
    orbit_ui_key(&ui, 1);
    assert(ui.page == 2);
    orbit_ui_key(&ui, 0);
    assert(ui.stars == 1);
    save(argv[1], "character", &ui, &n);
    assert(lv_anim_count_running() > 0);
    for (unsigned i = 0; i < 30; i++)
        motion_frame(argv[1], "motion", i);
    assert(lv_anim_count_running() == 0);
    /* Repeated input enhances the running burst without rewinding it. */
    orbit_ui_key(&ui, 0);
    orbit_ui_draw(&ui, &n, 78, render_now);
    lv_tick_inc(160);
    lv_timer_handler();
    orbit_ui_key(&ui, 0);
    orbit_ui_draw(&ui, &n, 78, render_now);
    for (unsigned i = 0; i < 30; i++)
        motion_frame(argv[1], "combo", i);
    assert(lv_anim_count_running() == 0);
    /* Sustained input must keep emitting, and never rewind the active phase. */
    lv_obj_t *effect = NULL;
    for (unsigned i = 0; i < lv_obj_get_child_count(lv_screen_active()); i++) {
        lv_obj_t *obj = lv_obj_get_child(lv_screen_active(), i);
        if (lv_obj_get_x(obj) == 24 && lv_obj_get_y(obj) == 96)
            effect = obj;
    }
    assert(effect);
    unsigned moving = 0;
    const void *previous_frame = NULL;
    for (unsigned i = 0; i < 30; i++) {
        orbit_ui_key(&ui, 0);
        orbit_ui_draw(&ui, &n, 78, render_now);
        lv_tick_inc(100);
        lv_timer_handler();
        lv_refr_now(NULL);
        const void *frame = lv_obj_get_user_data(effect);
        if (frame != previous_frame)
            moving++;
        previous_frame = frame;
        assert(lv_anim_count_running() > 0);
    }
    assert(moving >= 25);
    for (unsigned i = 0; i < 30; i++) {
        lv_tick_inc(40);
        lv_timer_handler();
    }
    assert(lv_anim_count_running() == 0);
    puts("Sustained input: continuous phase progression and release completion PASS");
    for (unsigned i = 0; i < 90; i++) {
        if (i < 60 && i % 4 == 0) {
            orbit_ui_key(&ui, 0);
            orbit_ui_draw(&ui, &n, 78, render_now);
        }
        motion_frame(argv[1], "continuous", i);
    }
    assert(lv_anim_count_running() == 0);
    /* Rapid presses stay bounded; leaving the page cancels all object animations. */
    lv_obj_t *retained_brand = lv_obj_get_child(lv_screen_active(), 0);
    unsigned long input_pixels = flushed_pixels;
    for (unsigned i = 0; i < 80; i++) {
        orbit_ui_key(&ui, 0);
        orbit_ui_draw(&ui, &n, 78, render_now);
        assert(lv_obj_get_child(lv_screen_active(), 0) == retained_brand);
        lv_tick_inc(20);
        lv_timer_handler();
        lv_refr_now(NULL);
        assert(lv_anim_count_running() <= 8);
    }
    printf("80 rapid presses flushed %lu pixels\n", flushed_pixels - input_pixels);
    orbit_ui_key(&ui, 1);
    orbit_ui_draw(&ui, &n, 78, render_now);
    assert(lv_anim_count_running() == 0);
    orbit_ui_key(&ui, -1);
    orbit_ui_draw(&ui, &n, 78, render_now);
    assert(lv_anim_count_running() == 0);
    puts("Motion completion, rapid input and navigation cancellation PASS");
    orbit_ui_key(&ui, 2);
    assert(ui.settings);
    orbit_ui_key(&ui, 1);
    assert(orbit_ui_key(&ui, 0) == 2 && ui.sound);
    save(argv[1], "settings", &ui, &n);
    orbit_ui_t safe = {.settings = true};
    assert(orbit_ui_key(&safe, 0) == 0 && safe.confirm_action == 1 && safe.settings);
    assert(orbit_ui_key(&safe, 0) == 0 && safe.confirm_action == 0 && safe.settings);
    assert(orbit_ui_key(&safe, 0) == 0 && safe.confirm_action == 1);
    assert(orbit_ui_key(&safe, 1) == 0 && safe.confirm_yes);
    assert(orbit_ui_key(&safe, 0) == 1 && !safe.settings && !safe.confirm_action);

    safe = (orbit_ui_t){.settings = true, .settings_row = 2};
    safe.quota.paired = true;
    assert(orbit_ui_key(&safe, 0) == 0 && safe.confirm_action == 2);
    assert(orbit_ui_key(&safe, 2) == 0 && safe.confirm_action == 0 && safe.settings);
    assert(orbit_ui_key(&safe, 0) == 0 && safe.confirm_action == 2);
    assert(orbit_ui_key(&safe, 1) == 0 && safe.confirm_yes);
    assert(orbit_ui_key(&safe, 0) == 7 && !safe.confirm_action);
    puts("Wi-Fi setup and Mac unpair confirmation, default-cancel and long-press cancel PASS");
    orbit_ui_key(&ui, 2);
    ui.page = 0;
    n.link = NET_SETUP;
    n.setup = true;
    strcpy(n.ap_name, "Starshore-DEMO");
    strcpy(n.ap_password, "DEMO12345678");
    save(argv[1], "setup", &ui, &n);
    n.link = NET_BAD_WIFI;
    n.wifi_reason = 201;
    save(argv[1], "bad-wifi", &ui, &n);
    n.wifi_reason = 202;
    save(argv[1], "auth-failure", &ui, &n);
    n.wifi_reason = 10000;
    save(argv[1], "dhcp-timeout", &ui, &n);
    n.setup = false;
    n.link = NET_OFFLINE;
    save(argv[1], "offline", &ui, &n);
    n.link = NET_ONLINE;
    n.snapshot.healthy = false;
    save(argv[1], "stale", &ui, &n);
    n.snapshot.healthy = true;
    n.snapshot.kind = ORBIT_FORECAST;
    save(argv[1], "forecast", &ui, &n);
    ui.page = 1;
    save(argv[1], "forecast-letter", &ui, &n);
    ui.page = 0;
    n.snapshot.time_kind = ORBIT_TIME_EXPECTED;
    n.snapshot.expected = render_now + 3600;
    save(argv[1], "forecast-expected", &ui, &n);
    ui.page = 1;
    save(argv[1], "forecast-expected-letter", &ui, &n);
    ui.page = 0;
    n.snapshot.time_kind = ORBIT_TIME_DEADLINE;
    n.snapshot.window_end = render_now + 7200;
    save(argv[1], "forecast-deadline", &ui, &n);
    ui.page = 1;
    save(argv[1], "forecast-deadline-letter", &ui, &n);
    ui.page = 0;
    n.snapshot.time_kind = ORBIT_TIME_WINDOW;
    n.snapshot.window_start = render_now + 1800;
    save(argv[1], "forecast-window", &ui, &n);
    ui.page = 1;
    save(argv[1], "forecast-window-letter", &ui, &n);
    ui.page = 0;
    n.snapshot.published = render_now - 10800;
    n.snapshot.window_start = render_now - 7200;
    n.snapshot.window_end = render_now - 3600;
    save(argv[1], "forecast-window-overdue", &ui, &n);
    ui.page = 1;
    save(argv[1], "forecast-window-overdue-letter", &ui, &n);
    ui.page = 0;
    n.snapshot.time_kind = ORBIT_TIME_EXPECTED;
    n.snapshot.expected = render_now - 3600;
    save(argv[1], "forecast-overdue", &ui, &n);
    ui.page = 1;
    save(argv[1], "forecast-overdue-letter", &ui, &n);
    ui.page = 0;
    n.snapshot.kind = ORBIT_COMPLETED;
    n.snapshot.effective = render_now - 1500;
    save(argv[1], "completed-time", &ui, &n);
    n.snapshot.kind = ORBIT_EMPTY;
    n.snapshot.published = n.snapshot.effective = n.snapshot.expected = 0;
    n.snapshot.window_start = n.snapshot.window_end = 0;
    save(argv[1], "empty", &ui, &n);
    n.link = NET_SYNCING;
    save(argv[1], "loading", &ui, &n);
    memset(&n.snapshot, 0, sizeof(n.snapshot));
    n.link = NET_OFFLINE;
    save(argv[1], "offline-first-sync", &ui, &n);
    n.link = NET_SETUP_EXPIRED;
    save(argv[1], "setup-expired", &ui, &n);
    ui.page = 2; n.link = NET_ONLINE;
    ui.quota.paired = true; ui.quota.valid = true; ui.quota.updated = render_now;
    ui.quota.windows[0] = (orbit_quota_window_t){10080, 42, render_now + 3600};
    save(argv[1], "book-first-sync", &ui, &n);
    for (unsigned i = 0; i < 12; ++i) motion_frame(argv[1], "book-load", i);
    save(argv[1], "quota-week-only", &ui, &n);
    unsigned char book_before[sizeof(pixels)]; memcpy(book_before, pixels, sizeof(pixels));
    const unsigned book_values[] = {100, 55, 15, 0};
    const char *book_names[] = {"book-full", "book-half", "book-low", "book-empty"};
    for (unsigned v = 0; v < 4; ++v) {
        ui.quota.windows[0].remaining = book_values[v];
        orbit_ui_draw(&ui, &n, render_battery, render_now);
        for (unsigned i = 0; i < 12; ++i) motion_frame(argv[1], book_names[v], i);
        save(argv[1], book_names[v], &ui, &n);
        bool changed = false;
        for (int y = 164; y < 180; ++y)
            for (int x = 148; x < 159; ++x)
                if (memcmp(pixels + (y * 240 + x) * 3, book_before + (y * 240 + x) * 3, 3)) changed = true;
        assert(changed);
        /* Quota transitions must leave the character, gold rim and paper untouched. */
        for (int y = 0; y < 257; ++y) for (int x = 0; x < 240; ++x) {
            if (!memcmp(pixels + (y * 240 + x) * 3, book_before + (y * 240 + x) * 3, 3)) continue;
            bool inset = false;
            for (unsigned i = 0; i < sizeof(orbit_book_pixels) / sizeof(orbit_book_pixels[0]); ++i)
                if (x == 148 + orbit_book_pixels[i].x && y == 164 + orbit_book_pixels[i].y) inset = true;
            assert(inset);
        }
        memcpy(book_before, pixels, sizeof(pixels));
    }
    puts("Book actual-value transitions, partial/full redraw equivalence and empty quota PASS");
    ui.quota.windows[0].remaining = 42;
    orbit_ui_draw(&ui, &n, render_battery, render_now);
    for (unsigned i = 0; i < 12; ++i) motion_frame(argv[1], "book-restore", i);
    ui.quota.windows[1] = (orbit_quota_window_t){300, 68, render_now + 1800};
    save(argv[1], "quota-two-windows", &ui, &n);
    ui.quota.updated = render_now - 720;
    save(argv[1], "quota-offline", &ui, &n);
    ui.pairing = true; ui.quota.paired = false; ui.quota.pairing = true; ui.quota.offer = true;
    save(argv[1], "quota-pair-confirm", &ui, &n);
    ui.quota.paired = true;
    save(argv[1], "quota-revoke-confirm", &ui, &n);
    ui.pairing = false; memset(&ui.quota, 0, sizeof(ui.quota));
    ui.page = 0;
    for (unsigned i = 0; i < 200; i++) {
        ui.source = i % 2;
        orbit_ui_draw(&ui, &n, 78, 1791883200);
        lv_refr_now(NULL);
    }
    lv_mem_monitor_t m;
    lv_mem_monitor(&m);
    printf("200 redraws PASS; LVGL used %u%%, largest free %zu bytes\n", m.used_pct,
           m.free_biggest_size);
    return 0;
}
