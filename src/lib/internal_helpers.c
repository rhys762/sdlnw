#include "internal_helpers.h"
#include "src/lib/SDLNW.h"

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    (void)w; // unused
    (void)renderer; // unused
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->size = *rect;
}

static void click(SDLNW_Widget* w, int x, int y) {
    (void)w; // unused
    (void)x; // unused
    (void)y; // unused
}

static void destroy(SDLNW_Widget* w) {
    free(w->data);
    w->data = NULL;
}

static SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    (void)w; // unused
    (void)x; // unused
    (void)y; // unused
    return SDL_SYSTEM_CURSOR_ARROW;
}

static void init_default_vtable(SDLNW_Widget_VTable* table) {
    table->draw = draw;
    table->size = size;
    table->click = click;
    table->destroy = destroy;
    table->appropriate_cursor = appropriate_cursor;
}

// init default widget
SDLNW_Widget* create_default_widget(void) {
    SDLNW_Widget* p = malloc(sizeof(SDLNW_Widget));
    
    *p = (SDLNW_Widget){0};

    init_default_vtable(&p->vtable);

    return p;
}

int is_point_within_rect(int x, int y, const SDL_Rect* rect) {
    return x >= rect->x && x < rect->x + rect->w &&
        y >= rect->y && y < rect->y + rect->h;
}
