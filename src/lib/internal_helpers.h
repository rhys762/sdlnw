#ifndef INTERNAL_HELPERS_H
#define INTERNAL_HELPERS_H

#include "SDLNW.h"

// init default widget
SDLNW_Widget* create_default_widget(void);

// inits a vtable to safe defaults
// void init_default_vtable(SDLNW_Widget_VTable* table);

// returns 1 if a point is in a rect, else 0
int is_point_within_rect(int x, int y, const SDL_Rect* rect);

#endif