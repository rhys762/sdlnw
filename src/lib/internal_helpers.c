#include "internal_helpers.h"

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->size = *rect;
}

static void destroy(SDLNW_Widget* w) {
}

void init_default_vtable(SDLNW_Widget_VTable* table) {
    table->draw = draw;
    table->size = size;
    table->destroy = destroy;
}
