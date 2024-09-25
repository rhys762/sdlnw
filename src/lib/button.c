#include "SDLNW.h"
#include "internal_helpers.h"

struct button_data {
    SDLNW_Widget* child;
    void* data;
    void(*cb)(void* data, int x, int y);
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct button_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void click(SDLNW_Widget* w, int x, int y) {
    struct button_data* data = w->data;

    if (is_point_within_rect(x, y, &w->size)) {
        data->cb(data->data, x, y);
    }
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct button_data* data = w->data;

    w->size = *rect;
    SDLNW_Widget_Size(data->child, rect);
}

static SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    (void)w; // unused
    (void)x; // unused
    (void)y; // unused
    return SDL_SYSTEM_CURSOR_HAND;
}

static void destroy(SDLNW_Widget* w) {
    struct button_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);
    data->child = NULL;

    data->data = NULL;
}

SDLNW_Widget* SDLNW_CreateButtonWidget(SDLNW_Widget* child, void* data, void(*cb)(void* data, int x, int y)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.click = click;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    widget->vtable.destroy = destroy;

    widget->data = malloc(sizeof(struct button_data));
    *((struct button_data*)widget->data) = (struct button_data){ .child = child, .data = data, .cb = cb};

    return widget;
}