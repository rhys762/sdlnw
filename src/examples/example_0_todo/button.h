#ifndef EXAMPLE_0_TODO_BUTTON_H
#define EXAMPLE_0_TODO_BUTTON_H

#include "SDLNW.h"
#include "misc.h"

SDLNW_Widget* create_button(const char* text, void* cb_data, void (*cb)(void*cb_data));

#endif