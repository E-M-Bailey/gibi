// Global settings
// In the future, this may be refactored into a non-global context struct.

#ifndef GIBI_SETTINGS_H
#define GIBI_SETTINGS_H

#include <stdlib.h>

// settings.c doesn't have to re-declare these settings without extern but no ODR violations occur.
#ifdef GIBI_SETTINGS_NO_EXTERN
#define DECL_SETTING(decl, def) decl def;
#else
#define DECL_SETTING(decl, def) extern decl;
#endif

// TODO add persistence

// Theme settings

// Miscellaneous display settings
// Gap between tab stops. Must always be positive.
DECL_SETTING(size_t tab_width, = 8);

#endif

