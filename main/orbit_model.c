#define _POSIX_C_SOURCE 200809L

#include "orbit_language.h"
#include "orbit_model.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { ROOT = 1, INSIGHTS, EVENTS, EVENT, URLS, OTHER };
/* States: object key/end, colon, value, comma/end; array value/end, comma/end.
 * After commas a separate state forbids a trailing comma. Long irrelevant strings
 * are discarded, so the 160+ KB public feed never occupies a full RAM buffer. */
enum { KEY, COLON, VALUE, COMMA, ARRAY_VALUE, ARRAY_COMMA, KEY_REQUIRED, ARRAY_REQUIRED };
static bool whitespace(unsigned char c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; }
static bool equal(const char *a, const char *b) { return strcmp(a, b) == 0; }
static void copy(char *dst, size_t n, const char *src) { snprintf(dst, n, "%s", src); }
int64_t orbit_iso_time(const char *s) {
    int y, m, d, h, min, sec, n = 0;
    if (sscanf(s, "%4d-%2d-%2dT%2d:%2d:%2d%n", &y, &m, &d, &h, &min, &sec, &n) != 6 || n != 19)
        return 0;
    if (y < 2020 || y > 2099 || m < 1 || m > 12 || h < 0 || h > 23 || min < 0 || min > 59 ||
        sec < 0 || sec > 59)
        return 0;
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max = days[m - 1] + (m == 2 && y % 4 == 0);
    if (d < 1 || d > max)
        return 0;
    const char *tail = s + n;
    if (*tail == '.') {
        tail++;
        if (!isdigit((unsigned char)*tail))
            return 0;
        while (isdigit((unsigned char)*tail))
            tail++;
    }
    if (!equal(tail, "Z"))
        return 0;
    int64_t day = 0;
    for (int a = 1970; a < y; a++)
        day += 365 + (a % 4 == 0 && (a % 100 != 0 || a % 400 == 0));
    for (int a = 1; a < m; a++)
        day += days[a - 1] + (a == 2 && y % 4 == 0);
    return (day + d - 1) * 86400 + h * 3600 + min * 60 + sec;
}
static bool safe_url(const char *s) {
    if (strncmp(s, "https://x.com/", 14) != 0 || strlen(s) >= 160)
        return false;
    for (; *s; s++)
        if (!(isalnum((unsigned char)*s) || strchr(":/._-", *s)))
            return false;
    return true;
}
static void accept_event(orbit_parser_t *p) {
    if (p->event_fields != 31 || !equal(p->event_type, "reset"))
        return;
    if (equal(p->event_state, "completed"))
        p->current.kind = ORBIT_COMPLETED;
    else if (equal(p->event_state, "scheduled") || equal(p->event_state, "announced"))
        p->current.kind = ORBIT_FORECAST;
    else
        return;
    if (p->current.published > p->result.published) {
        int64_t checked = p->result.checked;
        bool healthy = p->result.healthy;
        p->result = p->current;
        p->result.checked = checked;
        p->result.healthy = healthy;
    }
}
static bool scalar(orbit_parser_t *p, bool string) {
    if (!p->depth)
        return false;
    orbit_frame_t *f = &p->frames[p->depth - 1];
    if (f->state == KEY || f->state == KEY_REQUIRED) {
        if (!string)
            return false;
        copy(f->key, sizeof(f->key), p->overflow ? "" : p->token);
        f->state = COLON;
        return true;
    }
    if (f->state != VALUE && f->state != ARRAY_VALUE && f->state != ARRAY_REQUIRED)
        return false;
    f->state = f->array ? ARRAY_COMMA : COMMA;
    if (p->overflow)
        return true;
    const char *v = p->token;
    if (f->role == INSIGHTS) {
        if (equal(f->key, "schema_version"))
            p->schema = !string && equal(v, "1") ? 1 : 0;
        if (equal(f->key, "source_healthy")) {
            p->healthy_seen = !string && (equal(v, "true") || equal(v, "false"));
            p->result.healthy = !string && equal(v, "true");
        }
        if (equal(f->key, "source_checked_at") && string)
            p->result.checked = orbit_iso_time(v);
    } else if (f->role == EVENT && string) {
        if (equal(f->key, "event_id") && *v && strlen(v) < 32) {
            copy(p->current.id, 32, v);
            p->event_fields |= 1;
        }
        if (equal(f->key, "type")) {
            copy(p->event_type, 16, v);
            p->event_fields |= 2;
        }
        if (equal(f->key, "state")) {
            copy(p->event_state, 16, v);
            p->event_fields |= 4;
        }
        if (equal(f->key, "published_at")) {
            p->current.published = orbit_iso_time(v);
            if (p->current.published)
                p->event_fields |= 8;
        }
        if (equal(f->key, "effective_at"))
            p->current.effective = orbit_iso_time(v);
        if (equal(f->key, "expected_at"))
            p->current.expected = orbit_iso_time(v);
        if (equal(f->key, "window_start"))
            p->current.window_start = orbit_iso_time(v);
        if (equal(f->key, "window_end"))
            p->current.window_end = orbit_iso_time(v);
        if (equal(f->key, "time_kind"))
            p->current.time_kind = equal(v, "expected")   ? ORBIT_TIME_EXPECTED
                                   : equal(v, "start")    ? ORBIT_TIME_START
                                   : equal(v, "deadline") ? ORBIT_TIME_DEADLINE
                                   : equal(v, "window")   ? ORBIT_TIME_WINDOW
                                                          : ORBIT_TIME_UNKNOWN;
        if (equal(f->key, "revision") && strlen(v) < 32)
            copy(p->current.revision, 32, v);
    } else if (f->role == URLS && string && !p->current.url[0] && safe_url(v)) {
        copy(p->current.url, sizeof(p->current.url), v);
        p->event_fields |= 16;
    }
    return true;
}
static bool start(orbit_parser_t *p, bool array) {
    unsigned role = OTHER;
    if (!p->depth) {
        if (p->done || array)
            return false;
        role = ROOT;
    } else {
        orbit_frame_t *f = &p->frames[p->depth - 1];
        if (f->state != VALUE && f->state != ARRAY_VALUE && f->state != ARRAY_REQUIRED)
            return false;
        f->state = f->array ? ARRAY_COMMA : COMMA;
        if (f->role == ROOT && equal(f->key, "publicInsights") && !array) {
            if (p->insights)
                return false;
            p->insights = true;
            role = INSIGHTS;
        } else if (f->role == INSIGHTS && equal(f->key, "events") && array) {
            if (p->events)
                return false;
            p->events = true;
            role = EVENTS;
        } else if (f->role == EVENTS && !array) {
            role = EVENT;
            memset(&p->current, 0, sizeof(p->current));
            p->event_fields = 0;
            p->event_type[0] = p->event_state[0] = 0;
        } else if (f->role == EVENT && equal(f->key, "source_urls") && array)
            role = URLS;
    }
    if (p->depth >= 24)
        return false;
    p->frames[p->depth++] =
        (orbit_frame_t){.role = role, .state = array ? ARRAY_VALUE : KEY, .array = array};
    return true;
}
static bool number(const char *s) {
    if (*s == '-')
        s++;
    if (*s == '0')
        s++;
    else {
        if (*s < '1' || *s > '9')
            return false;
        while (isdigit((unsigned char)*s))
            s++;
    }
    if (*s == '.') {
        s++;
        if (!isdigit((unsigned char)*s))
            return false;
        while (isdigit((unsigned char)*s))
            s++;
    }
    if (*s == 'e' || *s == 'E') {
        s++;
        if (*s == '+' || *s == '-')
            s++;
        if (!isdigit((unsigned char)*s))
            return false;
        while (isdigit((unsigned char)*s))
            s++;
    }
    return !*s;
}
static void append(orbit_parser_t *p, char c) {
    if (p->length < sizeof(p->token) - 1)
        p->token[p->length++] = c;
    else
        p->overflow = true;
}
static bool step(orbit_parser_t *p, unsigned char c) {
    if (p->mode == 1) {
        if (p->unicode_left) {
            int v = isdigit(c)             ? c - '0'
                    : c >= 'a' && c <= 'f' ? c - 'a' + 10
                    : c >= 'A' && c <= 'F' ? c - 'A' + 10
                                           : -1;
            if (v < 0)
                return false;
            p->unicode_value = (p->unicode_value << 4) | (unsigned)v;
            if (!--p->unicode_left) {
                unsigned u = p->unicode_value;
                if (u == 0)
                    p->overflow = true;
                else if (u < 128)
                    append(p, (char)u);
                else { /* Relevant fields are ASCII; reject encoded non-ASCII values. */
                    p->overflow = true;
                }
            }
            return true;
        }
        if (p->escaped) {
            p->escaped = 0;
            if (c == 'u') {
                p->unicode_left = 4;
                p->unicode_value = 0;
                return true;
            }
            const char *from = "\"\\/bfnrt", *to = "\"\\/\b\f\n\r\t", *v = strchr(from, c);
            if (!v)
                return false;
            append(p, to[v - from]);
            return true;
        }
        if (c == '\\') {
            p->escaped = 1;
            return true;
        }
        if (c == '"') {
            p->mode = 0;
            p->token[p->length] = 0;
            return scalar(p, true);
        }
        if (c < 32)
            return false;
        append(p, (char)c);
        return true;
    }
    if (p->mode == 2) {
        if (c != ',' && c != ']' && c != '}' && !whitespace(c)) {
            if (c < 32)
                return false;
            append(p, (char)c);
            return true;
        }
        p->token[p->length] = 0;
        p->mode = 0;
        if (p->overflow ||
            !(equal(p->token, "true") || equal(p->token, "false") || equal(p->token, "null") ||
              number(p->token)) ||
            !scalar(p, false))
            return false;
        return step(p, c);
    }
    if (whitespace(c))
        return true;
    if (c == '{' || c == '[')
        return start(p, c == '[');
    if (!p->depth)
        return false;
    orbit_frame_t *f = &p->frames[p->depth - 1];
    if (c == '}' || c == ']') {
        if (f->array != (c == ']'))
            return false;
        if (f->state != (f->array ? ARRAY_VALUE : KEY) &&
            f->state != (f->array ? ARRAY_COMMA : COMMA))
            return false;
        if (f->role == EVENT)
            accept_event(p);
        if (!--p->depth)
            p->done = true;
        return true;
    }
    if (c == ':') {
        if (f->state != COLON)
            return false;
        f->state = VALUE;
        return true;
    }
    if (c == ',') {
        if (f->state != COMMA && f->state != ARRAY_COMMA)
            return false;
        f->state = f->array ? ARRAY_REQUIRED : KEY_REQUIRED;
        return true;
    }
    p->length = 0;
    p->overflow = false;
    if (c == '"') {
        p->mode = 1;
        return true;
    }
    if (f->state != VALUE && f->state != ARRAY_VALUE && f->state != ARRAY_REQUIRED)
        return false;
    p->mode = 2;
    append(p, (char)c);
    return true;
}
void orbit_parser_init(orbit_parser_t *p) { memset(p, 0, sizeof(*p)); }
bool orbit_parser_feed(orbit_parser_t *p, const char *data, size_t length) {
    if (length > 524288 - p->total) {
        p->failed = true;
        return false;
    }
    p->total += length;
    for (size_t i = 0; i < length; i++)
        if (!step(p, (unsigned char)data[i])) {
            p->failed = true;
            return false;
        }
    return !p->failed;
}
bool orbit_parser_finish(orbit_parser_t *p, int64_t now, orbit_snapshot_t *out) {
    if (p->failed || !p->done || p->depth || p->mode || !p->insights || !p->events ||
        p->schema != 1 || !p->healthy_seen || !p->result.checked || p->result.checked > now + 300 ||
        p->result.published > now + 300)
        return false;
    /* Optional timestamps must never turn publication into an effective time. */
    if (p->result.effective > now + 300)
        p->result.effective = 0;
    if (p->result.window_start && p->result.window_end &&
        p->result.window_start >= p->result.window_end) {
        p->result.window_start = p->result.window_end = 0;
    }
    p->result.received = now;
    *out = p->result;
    return true;
}
bool orbit_is_stale(const orbit_snapshot_t *s, int64_t now) {
    return !s->healthy || now < s->checked - 300 || now - s->checked > 1800 ||
           now - s->received > 1800 || (s->kind != ORBIT_EMPTY && now - s->published > 7 * 86400);
}
bool orbit_should_notify(const orbit_snapshot_t *old, const orbit_snapshot_t *next, int64_t now) {
    return old->received && next->kind != ORBIT_EMPTY && !orbit_is_stale(next, now) &&
           next->published <= now && now - next->published < 1800 &&
           next->published >= old->published &&
           (!equal(old->id, next->id) || old->kind != next->kind);
}
static int hex(char c) {
    return c >= '0' && c <= '9'   ? c - '0'
           : c >= 'A' && c <= 'F' ? c - 'A' + 10
           : c >= 'a' && c <= 'f' ? c - 'a' + 10
                                  : -1;
}
static bool decode(const char *src, size_t n, char *dst, size_t cap) {
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        char c = src[i];
        if (c == '+')
            c = ' ';
        else if (c == '%') {
            if (i + 2 >= n || hex(src[i + 1]) < 0 || hex(src[i + 2]) < 0)
                return false;
            c = (char)(hex(src[i + 1]) * 16 + hex(src[i + 2]));
            i += 2;
        }
        if (!c || j + 1 >= cap)
            return false;
        dst[j++] = c;
    }
    dst[j] = 0;
    return true;
}
bool orbit_decode_form(const char *body, char ssid[33], char password[65]) {
    unsigned seen = 0;
    ssid[0] = password[0] = 0;
    while (*body) {
        const char *end = strchr(body, '&');
        if (!end)
            end = body + strlen(body);
        const char *eq = memchr(body, '=', (size_t)(end - body));
        if (!eq)
            return false;
        if (eq - body == 4 && !strncmp(body, "ssid", 4)) {
            if (seen & 1 || !decode(eq + 1, (size_t)(end - eq - 1), ssid, 33))
                return false;
            seen |= 1;
        } else if (eq - body == 8 && !strncmp(body, "password", 8)) {
            if (seen & 2 || !decode(eq + 1, (size_t)(end - eq - 1), password, 65))
                return false;
            seen |= 2;
        } else
            return false;
        body = *end ? end + 1 : end;
    }
    size_t n = strlen(password);
    return seen == 3 && *ssid && (n == 0 || (n >= 8 && n <= 63));
}

static void timing_line(char out[64], const char *label, int64_t value, const char *missing) {
    if (!value) {
        snprintf(out, 64, "%s  %s", label, missing);
        return;
    }
    time_t utc8 = (time_t)(value + 8 * 3600);
    struct tm tm;
    gmtime_r(&utc8, &tm);
    snprintf(out, 64, "%s  %02d/%02d %02d:%02d", label, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
             tm.tm_min);
}
void orbit_timing_lines(const orbit_snapshot_t *s, int64_t now, char lines[3][64]) {
    memset(lines, 0, 3 * 64);
    timing_line(lines[0], orbit_tr("发布"), s->published, orbit_tr("暂无公告"));
    if (!s->received) {
        snprintf(lines[0], 64, "%s", orbit_tr("发布  等待首次同步"));
        snprintf(lines[1], 64, "%s", orbit_tr("完成  尚无可用消息"));
        snprintf(lines[2], 64, "%s", orbit_tr("预计  尚无可用消息"));
    } else if (s->kind == ORBIT_COMPLETED) {
        timing_line(lines[1], orbit_tr("完成"), s->effective, orbit_tr("未单独公布"));
        snprintf(lines[2], 64, "%s", orbit_tr("下次  暂无新预告"));
    } else if (s->kind == ORBIT_FORECAST) {
        int64_t target = 0;
        if (s->time_kind == ORBIT_TIME_WINDOW && s->window_start && s->window_end) {
            timing_line(lines[1], orbit_tr("预计开始"), s->window_start, orbit_tr("未明确"));
            timing_line(lines[2], now > s->window_end ? orbit_tr("原预计结束") : orbit_tr("预计结束"), s->window_end,
                        orbit_tr("未明确"));
            return;
        }
        const char *label = orbit_tr("预计");
        if (s->time_kind == ORBIT_TIME_EXPECTED)
            target = s->expected;
        if (s->time_kind == ORBIT_TIME_START) {
            target = s->window_start;
            label = orbit_tr("预计开始");
        }
        if (s->time_kind == ORBIT_TIME_DEADLINE) {
            target = s->window_end;
            label = now > s->window_end ? orbit_tr("原预告最晚") : orbit_tr("预计最晚");
        }
        timing_line(lines[1], label, target, orbit_tr("尚未明确时间"));
        snprintf(lines[2], 64, "%s",
                 target && now > target ? orbit_tr("尚未收到完成公告") : orbit_tr("预告不代表已经完成"));
    } else {
        snprintf(lines[1], 64, "%s", orbit_tr("完成  暂无有效公告"));
        snprintf(lines[2], 64, "%s", orbit_tr("预计  暂无新预告"));
    }
}
