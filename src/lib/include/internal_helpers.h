#ifndef INTERNAL_HELPERS_H
#define INTERNAL_HELPERS_H

#include "SDLNW.h"

// init default widget
SDLNW_Widget* create_default_widget(void);

// returns 1 if a point is in a rect, else 0
bool is_point_within_rect(int x, int y, const SDL_Rect* rect);

// src_null_terminted is a null terminated array of widget pointers
// should determine its length and write it to target_length,
// allocate an array for that many widgets and copy in from src, write to target
void _sdlnw_copy_null_terminated(SDLNW_Widget** src_null_terminted, SDLNW_Widget*** target, size_t* target_length);

#endif
