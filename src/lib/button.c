#include "SDLNW.h"
#include "internal_helpers.h"

struct button_data {
    SDLNW_Widget* child;
    void* data;
    void(*cb)(void* data, int x, int y);
};

static void button_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct button_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void button_click(SDLNW_Widget* w, SDLNW_Event_Click* event, bool* allow_passthrough) {
    struct button_data* data = w->data;

    if (is_point_within_rect(event->x, event->y, &w->size)) {
        *allow_passthrough = 0;
        data->cb(data->data, event->x, event->y);
    }
}

static void button_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct button_data* data = w->data;

    w->size = *rect;
    SDLNW_Widget_Size(data->child, rect);
}

static SDL_SystemCursor button_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    (void)w; // unused
    (void)x; // unused
    (void)y; // unused
    return SDL_SYSTEM_CURSOR_HAND;
}

static SDLNW_SizeRequest button_get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    struct button_data* data = w->data;

    return SDLNW_Widget_GetRequestedSize(data->child, locked_dimension, dimension_pixels);
}

static void button_destroy(SDLNW_Widget* w) {
    struct button_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);
    data->child = NULL;

    data->data = NULL;

    free(w->data);
    w->data = NULL;
}

static void button_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct button_data* data = widget->data;
    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateButtonWidget(SDLNW_Widget* child, void* data, void(*cb)(void* data, int x, int y)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = button_draw;
    widget->vtable.size = button_size;
    widget->vtable.click = button_click;
    widget->vtable.appropriate_cursor = button_appropriate_cursor;
    widget->vtable.destroy = button_destroy;
    widget->vtable.get_requested_size = button_get_requested_size;
    widget->vtable.trickle_down_event = button_trickle_down_event;

    widget->data = malloc(sizeof(struct button_data));
    *((struct button_data*)widget->data) = (struct button_data){ .child = child, .data = data, .cb = cb};

    return widget;
}