#pragma once
#include <stddef.h>
typedef enum { ORBIT_ZH_CN, ORBIT_ZH_HANT, ORBIT_EN, ORBIT_LANGUAGE_COUNT } orbit_language_t;
void orbit_language_set(unsigned language);
unsigned orbit_language_get(void);
const char *orbit_tr(const char *source);
