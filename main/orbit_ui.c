#include "orbit_language.h"
#include "orbit_ui.h"
#include "orbit_book_mask.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef ESP_PLATFORM
#include "esp_timer.h"
#include "esp_log.h"
#endif
LV_FONT_DECLARE(orbit_font_12);
LV_FONT_DECLARE(orbit_font_tc_12);
static const lv_font_t *font_12(void) { return orbit_language_get() == ORBIT_ZH_HANT ? &orbit_font_tc_12 : &orbit_font_12; }
LV_FONT_DECLARE(orbit_font_14);
LV_FONT_DECLARE(orbit_font_tc_14);
static const lv_font_t *font_14(void) { return orbit_language_get() == ORBIT_ZH_HANT ? &orbit_font_tc_14 : &orbit_font_14; }
LV_FONT_DECLARE(orbit_font_24);
LV_FONT_DECLARE(orbit_font_tc_24);
static const lv_font_t *font_24(void) { return orbit_language_get() == ORBIT_ZH_HANT ? &orbit_font_tc_24 : &orbit_font_24; }
LV_FONT_DECLARE(orbit_font_16);
LV_FONT_DECLARE(orbit_font_tc_16);
static const lv_font_t *font_16(void) { return orbit_language_get() == ORBIT_ZH_HANT ? &orbit_font_tc_16 : &orbit_font_16; }
LV_FONT_DECLARE(orbit_font_20);
LV_FONT_DECLARE(orbit_font_tc_20);
static const lv_font_t *font_20(void) { return orbit_language_get() == ORBIT_ZH_HANT ? &orbit_font_tc_20 : &orbit_font_20; }
LV_IMAGE_DECLARE(orbit_archivist);
LV_IMAGE_DECLARE(orbit_archivist_small);
LV_IMAGE_DECLARE(orbit_miniapp);
#define INK 0xF5ECF6
#define MUTED 0xBDAFC9
#define GOLD 0xE2C18B
#define BG 0x211A2B
#define PAPER 0xF1EADF
#define PLUM 0x43344B
#define PAPER_MUTED 0x786F79
static lv_obj_t *screen;
static unsigned rendered_stars;
static bool retained_character;
static int retained_battery;
static lv_obj_t *battery_fill, *battery_unknown;
static lv_obj_t *star_count, *orbit_stars[3], *celebration;
static unsigned celebration_level;
static int celebration_step;
static unsigned celebration_index;
static lv_obj_t *quota_status, *quota_age, *quota_number, *quota_bar, *quota_track, *book_overlay;
static uint8_t book_pixels[11 * 16 * 4];
static const lv_image_dsc_t book_image = {
    .header = {.magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_ARGB8888,
               .w = 11, .h = 16, .stride = 44},
    .data_size = sizeof(book_pixels), .data = book_pixels};
static int book_amount = -1, book_target = -1;
static bool book_fresh;
static void quota_refresh(const orbit_quota_t *q, int64_t now);
static void book_animate(void *obj, int32_t amount);
static uint32_t celebration_until;
static void celebration_frame(void *obj, int32_t value);
extern const lv_image_dsc_t *orbit_celebration_frame(unsigned level, int index);
extern unsigned orbit_celebration_index(unsigned level, int index);
extern uint16_t orbit_celebration_dirty(unsigned before, unsigned after);
static lv_obj_t *label(int x, int y, int w, const char *text, uint32_t color) {
    lv_obj_t *o = lv_label_create(screen);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_width(o, w);
    lv_obj_set_style_text_font(o, font_16(), 0);
    lv_obj_set_style_text_color(o, lv_color_hex(color), 0);
    lv_label_set_text(o, text);
    return o;
}
static lv_obj_t *box(lv_obj_t *parent, int x, int y, int w, int h, int radius, uint32_t color) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(o, radius, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}
/* Battery SOC is device power, never personal Codex quota. No charging flag is available. */
static void battery_update(int percent, uint32_t ink) {
    int width = percent < 0 ? 0 : (percent > 100 ? 100 : percent) * 21 / 100;
    if (percent > 0 && width == 0) width = 1;
    lv_obj_set_width(battery_fill, width ? width : 1);
    lv_obj_set_style_bg_color(battery_fill, lv_color_hex(percent <= 20 ? 0xC47763 : ink), 0);
    if (width) lv_obj_remove_flag(battery_fill, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(battery_fill, LV_OBJ_FLAG_HIDDEN);
    if (percent < 0) lv_obj_remove_flag(battery_unknown, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(battery_unknown, LV_OBJ_FLAG_HIDDEN);
}
static void battery_icon(int percent, uint32_t ink) {
    lv_obj_t *body = box(screen, 191, 15, 27, 14, 3, BG);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 1, 0);
    lv_obj_set_style_border_color(body, lv_color_hex(ink), 0);
    box(screen, 219, 19, 2, 6, 1, ink);
    battery_fill = box(screen, 194, 18, 1, 8, 1, ink);
    battery_unknown = label(199, 14, 11, "?", ink);
    lv_obj_set_style_text_font(battery_unknown, font_12(), 0);
    battery_update(percent, ink);
}
static void center(int y, const char *text, uint32_t color) {
    lv_obj_t *o = label(18, y, 204, text, color);
    lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_CENTER, 0);
}
static void centered_font(int y, const char *text, uint32_t color, const lv_font_t *font) {
    lv_obj_t *o = label(14, y, 212, text, color);
    lv_obj_set_style_text_font(o, font, 0);
    lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_CENTER, 0);
}
/* Reuse the product's existing artwork from Flash at its native prepared size. */
static lv_obj_t *archivist(int y) {
    box(screen, 58, y + 32, 124, 124, 62, 0x2E243B);
    lv_obj_t *o = lv_image_create(screen);
    lv_image_set_src(o, &orbit_archivist);
    lv_obj_set_pos(o, (240 - orbit_archivist.header.w) / 2, y);
    return o;
}
/* Object-bound animations are cancelled by LVGL when the page is rebuilt. */
static void animate(lv_obj_t *obj, lv_anim_exec_xcb_t exec, int from, int to,
                    unsigned duration, unsigned delay) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, exec);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_duration(&a, duration);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    if (exec == celebration_frame)
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}
static const lv_point_precise_t sparkle_points[] = {
    {6, 0}, {8, 4}, {12, 6}, {8, 8}, {6, 12}, {4, 8}, {0, 6}, {4, 4}, {6, 0}};
static lv_obj_t *sparkle(int x, int y, uint32_t color) {
    lv_obj_t *o = lv_line_create(screen);
    lv_line_set_points(o, sparkle_points, sizeof(sparkle_points) / sizeof(sparkle_points[0]));
    lv_obj_set_pos(o, x, y);
    lv_obj_set_style_line_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_line_width(o, 2, 0);
    return o;
}
static void celebration_draw(lv_event_t *e) {
    lv_draw_image_dsc_t d;
    lv_draw_image_dsc_init(&d);
    d.src = lv_obj_get_user_data(lv_event_get_target(e));
    lv_area_t area;
    lv_obj_get_coords(lv_event_get_target(e), &area);
    lv_draw_image(lv_event_get_layer(e), &d, &area);
}
static void celebration_frame(void *obj, int32_t value) {
    celebration_step = value;
    bool finished = (int32_t)(lv_tick_get() - celebration_until) >= 0;
    if (finished)
        lv_anim_delete(obj, celebration_frame);
    int frame = finished ? -1 : value;
    unsigned next = orbit_celebration_index(celebration_level, frame);
    uint16_t dirty = orbit_celebration_dirty(celebration_index, next);
    celebration_index = next;
    lv_obj_set_user_data(obj, (void *)orbit_celebration_frame(celebration_level, frame));
    lv_area_t origin;
    lv_obj_get_coords(obj, &origin);
    /* Invalidate only changed tiles; the draw callback obeys LVGL's clip area. */
    for (unsigned i = 0; i < 16; i++) {
        if (!(dirty & (1u << i)))
            continue;
        lv_area_t area = {.x1 = origin.x1 + (int)(i % 4) * 48,
                          .y1 = origin.y1 + (int)(i / 4) * 40};
        area.x2 = area.x1 + 47;
        area.y2 = area.y1 + 39;
        lv_obj_invalidate_area(obj, &area);
    }
}
#ifdef ESP_PLATFORM
/* Temporary aggregate diagnostics for hardware acceptance; no user data. */
static void motion_profile(lv_event_t *e) {
    static int64_t started, wait_started, last_started, render_sum, wait_sum, gap_sum;
    static unsigned samples, gap_samples;
    lv_event_code_t code = lv_event_get_code(e);
    int64_t tick = esp_timer_get_time();
    if (code == LV_EVENT_RENDER_START) {
        started = retained_character && lv_anim_count_running() ? tick : 0;
        if (started && last_started && tick - last_started < 200000) {
            gap_sum += tick - last_started;
            gap_samples++;
        }
        wait_started = 0;
        last_started = started;
    } else if (code == LV_EVENT_FLUSH_WAIT_START && started) {
        wait_started = tick;
    } else if (code == LV_EVENT_FLUSH_WAIT_FINISH && started && wait_started) {
        wait_sum += tick - wait_started;
        wait_started = 0;
    } else if (code == LV_EVENT_RENDER_READY && started) {
        render_sum += tick - started;
        if (++samples == 20) {
            ESP_LOGI("orbit_motion", "samples=20 render_avg_us=%lld wait_avg_us=%lld gap_avg_us=%lld",
                     (long long)(render_sum / 20), (long long)(wait_sum / 20),
                     (long long)(gap_samples ? gap_sum / gap_samples : 0));
            samples = gap_samples = 0;
            render_sum = wait_sum = gap_sum = 0;
            last_started = 0;
        }
        started = 0;
    }
}
#endif
static void star_response(unsigned stars, bool pressed) {
    char text[64];
    snprintf(text, sizeof(text), orbit_tr("本次点亮 %u 颗星"), stars);
    lv_label_set_text(star_count, text);
    for (unsigned i = 0; i < 3; i++) {
        bool lit = stars && i < ((stars - 1) % 3 + 1);
        lv_obj_set_style_line_color(orbit_stars[i], lv_color_hex(lit ? 0xFFE0A1 : 0x877594), 0);
        lv_anim_delete(orbit_stars[i], NULL);
        lv_obj_set_style_opa(orbit_stars[i], LV_OPA_COVER, 0);

    }
    if (!pressed)
        return;
    celebration_until = lv_tick_get() + 1120;
    bool running = lv_anim_get(celebration, celebration_frame) != NULL;
    celebration_level = running ? 1 : 0;
    if (running)
        celebration_frame(celebration, celebration_step);
    else
        animate(celebration, celebration_frame, 0, 23, 1120, 0);
}
static void star_scene(unsigned stars, bool pressed) {
    celebration = box(screen, 24, 60, 192, 160, 0, BG);
    lv_obj_set_user_data(celebration, (void *)orbit_celebration_frame(0, -1));
    lv_obj_add_event_cb(celebration, celebration_draw, LV_EVENT_DRAW_MAIN, NULL);
    celebration_index = 0;
    static const int positions[][2] = {{35, 123}, {167, 69}, {190, 180}};
    for (unsigned i = 0; i < 3; i++)
        orbit_stars[i] = sparkle(positions[i][0], positions[i][1], 0x877594);
    (void)stars;
    (void)pressed;
}
static void tabs(unsigned active, bool paper) {
    const char *names[] = {orbit_tr("首页"), orbit_tr("来信"), orbit_tr("角色")};
    uint32_t muted = paper ? PAPER_MUTED : MUTED;
    box(screen, 20, 292, 200, 1, 0, paper ? 0xD5CBC9 : 0x483951);
    for (unsigned i = 0; i < 3; i++) {
        if (i == active)
            box(screen, 18 + (int)i * 70, 295, 64, 24, 7, paper ? PLUM : GOLD);
        lv_obj_t *o = label(20 + (int)i * 70, 297, 60, names[i],
                            i == active ? (paper ? PAPER : BG) : muted);
        lv_obj_set_style_text_font(o, font_14(), 0);
        lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_CENTER, 0);
    }
}
/* Keep a common baseline and two columns, without a card around every fact. */
static void timing_row(int y, const char *line) {
    const char *value = strstr(line, "  ");
    if (!value || value - line > 15) {
        centered_font(y, line, PAPER_MUTED, font_14());
        return;
    }
    char name[32];
    size_t size = (size_t)(value - line);
    if (size >= sizeof(name))
        size = sizeof(name) - 1;
    memcpy(name, line, size);
    name[size] = 0;
    lv_obj_t *key = label(20, y + 2, 70, name, PAPER_MUTED);
    lv_obj_set_style_text_font(key, font_14(), 0);
    while (*value == ' ')
        value++;
    lv_obj_t *val = label(86, y, 134, value, PLUM);
    lv_obj_set_style_text_font(val, font_16(), 0);
    lv_obj_set_style_text_align(val, LV_TEXT_ALIGN_RIGHT, 0);
}
static void timestamp(char *out, size_t n, const char *prefix, int64_t value) {
    if (value < 1700000000) {
        snprintf(out, n, "%s --", prefix);
        return;
    }
    time_t t = (time_t)(value + 8 * 3600);
    struct tm tm;
    gmtime_r(&t, &tm);
    snprintf(out, n, "%s %02d/%02d %02d:%02d", prefix, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
             tm.tm_min);
}
int orbit_ui_key(orbit_ui_t *u, int key) {
    if (u->confirm_action) {
        if (key == 2) {
            u->confirm_action = 0;
            u->confirm_yes = false;
            return 0;
        }
        if (key) {
            u->confirm_yes = !u->confirm_yes;
            return 0;
        }
        if (!u->confirm_yes) {
            u->confirm_action = 0;
            return 0;
        }
        unsigned action = u->confirm_action;
        u->confirm_action = 0;
        u->confirm_yes = false;
        if (action == 1) {
            u->settings = false;
            return 1;
        }
        return action == 3 ? 8 : 7;
    }
    if (u->pairing) {
        if (key == 0 && (u->quota.offer || u->quota.paired)) return 4;
        if (key != 0) { u->pairing = false; return 5; }
        return 0;
    }
    if (key == 2) {
        if (u->source)
            u->source = false;
        else {
            u->settings = !u->settings;
            u->settings_row = 0;
        }
        return 0;
    }
    if (u->settings) {
        if (key)
            u->settings_row = (u->settings_row + (key > 0 ? 1 : 5)) % 6;
        else if (u->settings_row == 0) {
            u->settings = false;
        } else if (u->settings_row == 1) {
            u->confirm_action = 1;
            u->confirm_yes = false;
            return 0;
        } else if (u->settings_row == 2) {
            u->sound = !u->sound;
            return 2;
        } else if (u->settings_row == 3) {
            if (!u->wish.ready) return 0;
            if (u->wish.enabled) return 8;
            u->confirm_action = 3; u->confirm_yes = false; return 0;
        } else if (u->settings_row == 4) {
            if (u->quota.paired) {
                u->confirm_action = 2;
                u->confirm_yes = false;
                return 0;
            }
            u->settings = false; u->pairing = true; return 3;
        } else if (u->settings_row == 5) {
            u->language = (u->language + 1) % ORBIT_LANGUAGE_COUNT;
            return 6;
        } else
            u->settings = false;
        return 0;
    }
    if (u->source) {
        if (key == 0)
            u->source = false;
        return 0;
    }
    if (key)
        u->page = (u->page + (key > 0 ? 1 : 2)) % 3;
    else if (u->page == 0)
        u->page = 1;
    else if (u->page == 1)
        u->source = true;
    else {
        u->stars = (u->stars + 1) % 100;
        return 9;
    }
    return 0;
}
void orbit_ui_draw(const orbit_ui_t *u, const orbit_network_t *n, int battery, int64_t now) {
    unsigned before_language = orbit_language_get();
    orbit_language_set(u->language);
    /* A key press updates retained small objects, never the whole character page. */
    bool character = u->page == 2 && !u->settings && !u->source && !u->pairing && !n->setup;
    if (retained_character && character && before_language == u->language) {
        if (battery != retained_battery) {
            battery_update(battery, MUTED);
            retained_battery = battery;
        }
        if (u->stars != rendered_stars)
            star_response(u->stars, true);
        rendered_stars = u->stars;
        quota_refresh(&u->quota, now);
        return;
    }
    retained_character = false;
    if (screen) lv_anim_delete(screen, book_animate);
    if (!screen) {
        screen = lv_obj_create(NULL);
        lv_obj_remove_style_all(screen);
        lv_obj_set_style_bg_color(screen, lv_color_hex(BG), 0);
        lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
        lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        lv_screen_load(screen);
#ifdef ESP_PLATFORM
        lv_display_add_event_cb(lv_display_get_default(), motion_profile, LV_EVENT_ALL, NULL);
#endif
    } else
        lv_obj_clean(screen);
    bool letter = u->page == 1 && !u->settings && !u->source && !n->setup;
    lv_obj_set_style_bg_color(screen, lv_color_hex(letter ? PAPER : BG), 0);
    char text[96];
    if (u->language == ORBIT_EN) {
        lv_obj_t *brand = label(20, 9, 150, "Quota Orbit", letter ? PLUM : INK);
        lv_obj_set_style_text_font(brand, font_20(), 0);
    } else {
        lv_obj_t *brand_zh = label(20, 8, 84, orbit_tr("额度星盘"), letter ? PLUM : INK);
        lv_obj_set_style_text_font(brand_zh, font_20(), 0);
        lv_obj_t *brand_en = label(110, 16, 70, "Quota Orbit", letter ? PLUM : INK);
        lv_obj_set_style_text_font(brand_en, font_12(), 0);
        lv_obj_update_layout(screen);
        lv_obj_align_to(brand_en, brand_zh, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    }
    battery_icon(battery, letter ? PAPER_MUTED : MUTED);

    if (u->pairing) {
        center(60, u->quota.paired ? orbit_tr("解除 Mac 配对？") : u->quota.offer ? orbit_tr("允许这台 Mac 同步？") : orbit_tr("USB 连接 Mac"), GOLD);
        center(112, u->quota.paired ? orbit_tr("清除个人额度与授权") : orbit_tr("账号凭据留在电脑"), INK);
        center(150, u->quota.offer || u->quota.paired ? orbit_tr("确定允许 · 上下取消") : orbit_tr("打开 Mac 同步助手"), MUTED);
        center(190, u->quota.ip[0] ? u->quota.ip : orbit_tr("等待 Wi-Fi 连接"), MUTED);
        if (!u->quota.paired) center(238, orbit_tr("两分钟内完成配对"), MUTED);
        center(275, orbit_tr("长按确定返回"), MUTED);
        return;
    }

    if (u->confirm_action) {
        bool unpair = u->confirm_action == 2;
        if (u->confirm_action == 3) {
            center(57, orbit_tr("同步官网祈愿？"), GOLD);
            centered_font(98, orbit_tr("点星计入官网总次数"), INK, font_14());
            centered_font(122, orbit_tr("仅发送随机互动编号"), INK, font_14());
            centered_font(146, orbit_tr("不发送账号或个人额度"), INK, font_14());
            centered_font(170, orbit_tr("无需 Mac · 联网自动同步"), MUTED, font_12());
            box(screen, u->confirm_yes ? 126 : 20, 205, 88, 32, 8, 0x483951);
            label(40, 212, 56, orbit_tr("取消"), u->confirm_yes ? MUTED : GOLD);
            label(144, 212, 68, orbit_tr("开启"), u->confirm_yes ? GOLD : MUTED);
            centered_font(263, orbit_tr("上下选择 · 确定执行"), MUTED, font_12());
            centered_font(286, orbit_tr("长按确定返回"), MUTED, font_12());
            return;
        }
        center(70, orbit_tr(unpair ? "解除 Mac 配对？" : "重新配网？"), GOLD);
        centered_font(119, orbit_tr(unpair ? "将清除设备额度与授权" : "旧 Wi-Fi 保留至连接成功"), INK, font_14());
        box(screen, u->confirm_yes ? 126 : 20, 171, 88, 32, 8, 0x483951);
        label(40, 178, 56, orbit_tr("取消"), u->confirm_yes ? MUTED : GOLD);
        label(144, 178, 68, orbit_tr("确认"), u->confirm_yes ? GOLD : MUTED);
        center(243, orbit_tr("上下选择 · 确定执行"), MUTED);
        center(278, orbit_tr("长按确定返回"), MUTED);
        return;
    }

    if (u->settings) {
        center(46, orbit_tr("设置"), GOLD);
        const char *rows[] = {orbit_tr("返回"), orbit_tr("重新配网"), u->sound ? orbit_tr("提示音：开") : orbit_tr("提示音：关"), orbit_tr(u->wish.enabled ? "官网祈愿：开" : "官网祈愿：关"), u->quota.paired ? orbit_tr("解除 Mac 配对") : orbit_tr("连接 Mac"), orbit_tr(u->language == ORBIT_EN ? "语言：English" : u->language == ORBIT_ZH_HANT ? "语言：繁體中文" : "语言：简体中文")};
        for (unsigned i = 0; i < 6; i++) {
            if (u->settings_row == i)
                box(screen, 20, 68 + (int)i * 26, 200, 24, 8, 0x483951);
            label(32, 71 + (int)i * 26, 180, rows[i], INK);
        }
        if (u->settings_row == 3) {
            const char *status = u->wish.status == WISH_STORAGE_ERROR ? "保存失败 · 暂停上传" :
                u->wish.status == WISH_UNAVAILABLE ? "服务未就绪" :
                u->wish.status == WISH_FULL ? "部分点击未记录" :
                !u->wish.enabled ? "仅在本机点星" :
                u->wish.pending ? "等待联网同步" : "没有待同步记录";
            snprintf(text, sizeof(text), "%s · %u", orbit_tr(status), u->wish.pending);
            centered_font(235, text, MUTED, font_12());
            centered_font(253, orbit_tr("关闭清除待发，已发不可撤回"), MUTED, font_12());
            centered_font(273, orbit_tr("上下选择 · 确定执行"), MUTED, font_12());
        } else centered_font(261, orbit_tr("上下选择 · 确定执行"), MUTED, font_12());
        centered_font(291, orbit_tr("长按确定返回"), MUTED, font_12());
        return;
    }
    if (n->setup) {
        const char *setup_status =
            n->link == NET_CONNECTING ? orbit_tr("正在验证 Wi-Fi") : orbit_tr("手机连接设备热点");
        if (n->link == NET_BAD_WIFI) {
            const char *reason =
                n->wifi_reason == 10000   ? orbit_tr("未取得网络地址")
                : n->wifi_reason == 10001 ? orbit_tr("连接超时")
                : n->wifi_reason == 201   ? orbit_tr("未找到网络")
                : (n->wifi_reason == 202 || n->wifi_reason == 15 || n->wifi_reason == 204)
                    ? orbit_tr("网络认证失败")
                : (n->wifi_reason >= 210 && n->wifi_reason <= 212) ? orbit_tr("网络条件不兼容")
                                                                   : orbit_tr("连接失败");
            snprintf(text, sizeof(text), "%s E%u", reason, n->wifi_reason);
            setup_status = text;
        }
        center(54, setup_status, GOLD);
        center(87, n->ap_name, INK);
        snprintf(text, sizeof(text), orbit_tr("密码 %s"), n->ap_password);
        center(114, text, INK);
        center(149, orbit_tr("连接后用浏览器打开"), MUTED);
        center(176, "http://192.168.4.1", GOLD);
        center(211, orbit_tr("填写 2.4 GHz Wi-Fi"), INK);
        center(237, orbit_tr("成功后热点自动关闭"), MUTED);
        center(266, orbit_tr("10 分钟未完成会关闭"), MUTED);
        center(287, orbit_tr("长按确定打开设置"), MUTED);
        return;
    }
    if (u->source) {
        centered_font(41, orbit_tr("星岸重置助手"), GOLD, font_14());
        lv_obj_t *code = lv_image_create(screen);
        lv_image_set_src(code, &orbit_miniapp);
        lv_obj_set_pos(code, 16, 64);
        centered_font(278, "www.starshoreai.com/quota-orbit", INK, font_12());
        centered_font(301, orbit_tr("微信扫码 · 确定返回"), MUTED, font_12());
        return;
    }
    const char *status =
        n->link == NET_CONNECTING      ? orbit_tr("正在连接 Wi-Fi")
        : n->link == NET_SYNCING       ? orbit_tr("正在同步公开消息")
        : n->link == NET_CLOCK         ? orbit_tr("正在校准时间")
        : n->link == NET_SETUP_EXPIRED ? orbit_tr("配网超时，长按重试")
        : n->link == NET_ERROR         ? orbit_tr("设备异常，长按重新配网")
        : n->link == NET_OFFLINE
            ? (n->snapshot.received ? orbit_tr("同步中断 · 显示旧记录") : orbit_tr("暂未同步 · 稍后自动重试"))
        : orbit_is_stale(&n->snapshot, now) ? orbit_tr("来源过期 · 请查看依据")
                                            : orbit_tr("已同步 · 公开消息");
    if (n->notification_test[0]) status = n->notification_test;
    char timing[3][64];
    orbit_timing_lines(&n->snapshot, now, timing);
    const char *title = !n->snapshot.received                 ? orbit_tr("等待同步")
                        : n->snapshot.kind == ORBIT_COMPLETED ? orbit_tr("已宣布完成")
                        : n->snapshot.kind == ORBIT_FORECAST  ? orbit_tr("已预告")
                                                              : orbit_tr("暂无公告");
    const orbit_snapshot_t *snap = &n->snapshot;
    int64_t forecast_end = snap->time_kind == ORBIT_TIME_EXPECTED ? snap->expected :
        snap->time_kind == ORBIT_TIME_START ? snap->window_start : snap->window_end;
    bool overdue = snap->kind == ORBIT_FORECAST && forecast_end > 0 && now > forecast_end;
    const char *summary = !snap->received ? (n->link == NET_OFFLINE ? "暂未同步 · 稍后自动重试" : "首次连接，正在获取消息") :
        snap->kind == ORBIT_COMPLETED ? "公开重置已有完成公告" :
        snap->kind == ORBIT_FORECAST ? (overdue ? "预告时间已过，等待后续" : "公开来源已发布重置预告") :
        "目前没有可展示的重置消息";
    if (u->page == 0) {
        lv_obj_t *portrait = lv_image_create(screen);
        lv_image_set_src(portrait, &orbit_archivist_small);
        lv_obj_set_pos(portrait, 170, 48);
        lv_obj_t *eyebrow = label(20, 48, 145, orbit_tr("公开重置公告"), MUTED);
        lv_obj_set_style_text_font(eyebrow, font_14(), 0);
        lv_obj_t *heading = label(20, 78, 145, overdue ? orbit_tr("等待后续") : title, GOLD);
        lv_obj_set_style_text_font(heading, u->language == ORBIT_EN ? font_20() : font_24(), 0);
        lv_obj_t *note = label(20, 120, 145, orbit_tr("个人额度需核实"), MUTED);
        lv_obj_set_style_text_font(note, font_12(), 0);
        box(screen, 12, 154, 216, 132, 10, PAPER);
        centered_font(164, status, PAPER_MUTED, font_12());
        centered_font(187, orbit_tr(summary), PLUM, font_12());
        if (snap->received && snap->kind == ORBIT_FORECAST && !overdue) {
            timing_row(209, timing[1]);
            if (snap->time_kind == ORBIT_TIME_WINDOW) timing_row(232, timing[2]);
        } else {
            centered_font(213, orbit_tr("个人额度请到角色页查看"), PAPER_MUTED, font_12());
        }
        box(screen, 24, 258, 192, 1, 0, 0xD5CBC9);
        centered_font(266, orbit_tr("确定 · 查看来信"), PLUM, font_12());
    } else if (u->page == 1) {
        centered_font(39, status, PAPER_MUTED, font_12());
        const char *letter_title = !snap->received ? (n->link == NET_OFFLINE ? "等待同步" : "正在查收来信") :
            snap->kind == ORBIT_COMPLETED ? "完成公告解读" :
            snap->kind == ORBIT_FORECAST ? "预告解读" : "暂时没有来信";
        lv_obj_t *heading = label(20, 64, 200, orbit_tr(letter_title), PLUM);
        lv_obj_set_style_text_font(heading, font_20(), 0);
        lv_obj_t *note = label(20, 96, 200, orbit_tr("状态解读 · 非公告原文"), PAPER_MUTED);
        lv_obj_set_style_text_font(note, font_12(), 0);
        box(screen, 20, 119, 200, 1, 0, 0x9A7B55);
        lv_obj_t *body = label(20, 132, 200, orbit_tr(summary), PLUM);
        lv_obj_set_style_text_font(body, font_12(), 0);
        body = label(20, 154, 200, orbit_tr(!snap->received ? "同步成功后自动显示" :
            snap->kind == ORBIT_COMPLETED ? "各账号请以自己的额度为准" :
            snap->kind == ORBIT_FORECAST ? "尚未收到完成公告" : "收到新消息后自动显示"), PLUM);
        lv_obj_set_style_text_font(body, font_12(), 0);
        int meta_y = 199;
        if (snap->received && snap->kind == ORBIT_FORECAST) {
            timing_row(179, timing[1]);
            meta_y = 221;
            if (snap->time_kind == ORBIT_TIME_WINDOW) {
                timing_row(202, timing[2]);
                meta_y = 237;
            }
        } else if (snap->received && snap->kind == ORBIT_COMPLETED && snap->effective) {
            timing_row(179, timing[1]);
            meta_y = 221;
        }
        box(screen, 20, meta_y - 9, 200, 1, 0, 0xD5CBC9);
        if (snap->received && snap->published) {
            timestamp(text, sizeof(text), orbit_tr("发布"), snap->published);
            lv_obj_t *published = label(20, meta_y, 200, text, PAPER_MUTED);
            lv_obj_set_style_text_font(published, font_12(), 0);
        }
        timestamp(text, sizeof(text), orbit_tr("上次同步"), snap->received);
        lv_obj_t *updated = label(20, meta_y + 18, 200, text, PAPER_MUTED);
        lv_obj_set_style_text_font(updated, font_12(), 0);
        centered_font(275, orbit_tr("北京时间 · 确定打开小程序"), PAPER_MUTED, font_12());
    } else {
        centered_font(36, orbit_tr("月夜档案员"), GOLD, font_20());
        star_scene(u->stars, false);
        /* Scene, sparks and original book mask share the same vertical shift. */
        book_overlay = lv_image_create(screen);
        lv_image_set_src(book_overlay, &book_image);
        lv_obj_set_pos(book_overlay, 148, 128);
        book_amount = book_target = -2;
        star_count = label(14, 222, 212, "", MUTED);
        lv_obj_set_style_text_font(star_count, font_12(), 0);
        lv_obj_set_style_text_align(star_count, LV_TEXT_ALIGN_RIGHT, 0);
        star_response(u->stars, u->stars != rendered_stars);
        quota_status = label(20, 247, 200, "", MUTED);
        lv_obj_set_style_text_font(quota_status, font_12(), 0);
        quota_number = label(156, 239, 64, "", GOLD);
        lv_obj_set_style_text_font(quota_number, font_24(), 0);
        lv_obj_set_style_text_align(quota_number, LV_TEXT_ALIGN_RIGHT, 0);
        quota_track = box(screen, 20, 270, 200, 3, 1, 0x483951);
        quota_bar = box(screen, 20, 270, 1, 3, 1, GOLD);
        quota_age = label(20, 278, 200, "", MUTED);
        lv_obj_set_style_text_font(quota_age, font_12(), 0);
        quota_refresh(&u->quota, now);
    }
    retained_character = character;
    retained_battery = battery;
    rendered_stars = u->stars;
    tabs(u->page, u->page == 1);
}

static void book_animate(void *obj, int32_t amount) {
    (void)obj; book_amount = amount;
    uint32_t color = book_target >= 400 ? 0x70D7A3 : book_target >= 200 ? 0xF1B45E : 0xF17F83;
    memset(book_pixels, 0, sizeof(book_pixels));
    const int bottom[3] = {4, 10, 15}, height[3] = {4, 5, 5};
    for (unsigned i = 0; i < sizeof(orbit_book_pixels) / sizeof(orbit_book_pixels[0]); ++i) {
        const orbit_book_pixel_t *p = &orbit_book_pixels[i];
        int filled = amount < 0 ? 0 : amount * 3 - (2 - p->band) * 1000;
        if (filled < 0) filled = 0;
        if (filled > 1000) filled = 1000;
        bool lit = book_fresh && filled > 0 && (bottom[p->band] - p->y) * 1000 < height[p->band] * filled;
        uint32_t shade_color = lit ? color : 0x535966;
        unsigned offset = (p->y * 11 + p->x) * 4;
        book_pixels[offset] = (shade_color & 255) * p->shade / 255;
        book_pixels[offset + 1] = ((shade_color >> 8) & 255) * p->shade / 255;
        book_pixels[offset + 2] = ((shade_color >> 16) & 255) * p->shade / 255;
        book_pixels[offset + 3] = 255;
    }
    lv_obj_invalidate(book_overlay);
}

static void quota_refresh(const orbit_quota_t *q, int64_t now) {
    if (!quota_status) return;
    char text[120]; const orbit_quota_window_t *slots[2]; orbit_quota_slots(q, slots);
    int remaining = -1;
    for (unsigned i = 0; i < 2; ++i)
        if (slots[i] && (remaining < 0 || slots[i]->remaining < remaining)) remaining = slots[i]->remaining;
    bool fresh = orbit_quota_fresh(q, now);
    int target = remaining < 0 || !fresh ? -1 : remaining * 10;
    if (target != book_target || fresh != book_fresh) {
        book_fresh = fresh;
        lv_anim_delete(screen, book_animate);
        int start = book_amount < 0 ? 0 : book_amount;
        book_target = target;
        if (target < 0) book_animate(screen, -1);
        else if (target == start) book_animate(screen, target);
        else {
            lv_anim_t a; lv_anim_init(&a);
            lv_anim_set_var(&a, screen); lv_anim_set_exec_cb(&a, book_animate);
            lv_anim_set_values(&a, start, target); lv_anim_set_duration(&a, 420);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out); lv_anim_start(&a);
        }
    }
    char age_text[120];
    orbit_quota_summary(q, now, text, sizeof(text), age_text, sizeof(age_text));
    bool single = q->paired && q->valid && ((slots[0] != NULL) != (slots[1] != NULL));
    lv_label_set_text(quota_number, "");
    if (single) {
        /* Keep the translated period and remaining label; enlarge only its value. */
        char *value = strrchr(text, ' ');
        if (value) {
            lv_label_set_text(quota_number, value + 1);
            *value = 0;
        }
    }
    lv_label_set_text(quota_status, text);
    if (remaining < 0 || !q->paired) {
        lv_obj_add_flag(quota_track, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(quota_bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(quota_track, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_width(quota_bar, remaining > 0 ? remaining * 2 : 1);
        lv_obj_set_style_bg_color(quota_bar, lv_color_hex(fresh ? GOLD : MUTED), 0);
        if (remaining) lv_obj_remove_flag(quota_bar, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(quota_bar, LV_OBJ_FLAG_HIDDEN);
    }
    lv_label_set_text(quota_age, age_text);
}
