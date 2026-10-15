#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum { ORBIT_EMPTY, ORBIT_FORECAST, ORBIT_COMPLETED } orbit_kind_t;
typedef enum {
    ORBIT_TIME_UNKNOWN,
    ORBIT_TIME_EXPECTED,
    ORBIT_TIME_START,
    ORBIT_TIME_DEADLINE,
    ORBIT_TIME_WINDOW
} orbit_time_kind_t;
typedef struct {
    orbit_kind_t kind;
    char id[32], revision[32], url[160];
    int64_t published, checked, received;
    bool healthy;
    int64_t effective, expected, window_start, window_end;
    orbit_time_kind_t time_kind;
} orbit_snapshot_t;

typedef struct {
    char key[40];
    unsigned role, state;
    bool array;
} orbit_frame_t;
typedef struct {
    orbit_frame_t frames[24];
    unsigned depth, mode, escaped, unicode_left, unicode_value;
    char token[256];
    size_t length, total;
    bool overflow, failed, done, insights, events, healthy_seen;
    unsigned schema, event_fields;
    char event_type[16], event_state[16];
    orbit_snapshot_t result, current;
} orbit_parser_t;
void orbit_parser_init(orbit_parser_t *p);
bool orbit_parser_feed(orbit_parser_t *p, const char *data, size_t length);
bool orbit_parser_finish(orbit_parser_t *p, int64_t now, orbit_snapshot_t *out);
int64_t orbit_iso_time(const char *s);
bool orbit_is_stale(const orbit_snapshot_t *s, int64_t now);
bool orbit_should_notify(const orbit_snapshot_t *old, const orbit_snapshot_t *next, int64_t now);
bool orbit_decode_form(const char *body, char ssid[33], char password[65]);

void orbit_timing_lines(const orbit_snapshot_t *s, int64_t now, char lines[3][64]);
