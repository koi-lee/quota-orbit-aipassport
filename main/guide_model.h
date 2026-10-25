#pragma once
#include "guide_record.h"
typedef enum { GUIDE_VOICE_OFF, GUIDE_VOICE_STARTING, GUIDE_VOICE_PLAYING, GUIDE_VOICE_STOPPING, GUIDE_VOICE_ERROR } guide_voice_state_t;
typedef enum { GUIDE_COMMAND_NONE, GUIDE_COMMAND_PLAY, GUIDE_COMMAND_STOP } guide_command_t;
typedef enum { GUIDE_HOME, GUIDE_LIST, GUIDE_FILTER, GUIDE_READ, GUIDE_ACTIONS, GUIDE_INFO, GUIDE_RECORDS, GUIDE_COLLECTION, GUIDE_STATS, GUIDE_CLEAR, GUIDE_HELP, GUIDE_SOUND } guide_view_t;
typedef struct { guide_view_t view; uint8_t selected, park, filter, spot, page, collection, home_continue; } guide_nav_t;
typedef struct { guide_nav_t nav, stack[8]; unsigned depth; guide_record_t record; const char *notice; bool storage_ok; guide_voice_state_t voice; guide_command_t command; uint8_t volume; bool save_volume; } guide_model_t;
void guide_init(guide_model_t *m, guide_record_t r);
unsigned guide_count(const guide_model_t *m);
uint8_t guide_list_id(const guide_model_t *m, unsigned row);
const char *guide_item(const guide_model_t *m, unsigned row);
/* key: -1 up, +1 down, 0 OK, 2 long OK. Returns persistence requested. */
bool guide_key(guide_model_t *m, int key);
unsigned guide_bits(uint32_t bits);
