#include "internal_helpers.h"

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->size = *rect;
}

static void click(SDLNW_Widget* w, int x, int y) {
}

static void destroy(SDLNW_Widget* w) {
    free(w->data);
    w->data = NULL;
}

void init_default_vtable(SDLNW_Widget_VTable* table) {
    table->draw = draw;
    table->size = size;
    table->click = click;
    table->destroy = destroy;
}

int is_point_within_rect(int x, int y, const SDL_Rect* rect) {
    return x >= rect->x && x < rect->x + rect->w &&
        y >= rect->y && y < rect->y + rect->h;
}
