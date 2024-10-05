#include "SDLNW.h"
#include "internal_helpers.h"

struct composite_data {
    SDLNW_Widget* child;
    void* data;
    SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data);
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct composite_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct composite_data* data = w->data;

    w->size = *rect;

    SDLNW_Widget_Size(data->child, rect);
}

static SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct composite_data* data = w->data;
    return SDLNW_Widget_GetAppropriateCursor(data->child, x, y);
}

static void destroy(SDLNW_Widget* w) {
    struct composite_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);
}

void SDLNW_Widget_Recompose(SDLNW_Widget* w) {
    struct composite_data* data = w->data;

    if (data->child != NULL) {
        SDLNW_Widget_Destroy(data->child);
        data->child = NULL;
    }
    
    data->child = data->cb(w, data->data);
    SDLNW_Widget_Size(data->child, &w->size);
}

static SDLNW_SizeRequest get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    struct composite_data* data = w->data;

    return SDLNW_Widget_GetRequestedSize(data->child, locked_dimension, dimension_pixels);
}

static void trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, int* allow_passthrough) {
    struct composite_data* data = widget->data;

    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateCompositeWidget(void* data, SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    widget->vtable.destroy = destroy;
    widget->vtable.get_requested_size = get_requested_size;
    widget->vtable.trickle_down_event = trickle_down_event;

    widget->data = malloc(sizeof(struct composite_data));
    *((struct composite_data*)widget->data) = (struct composite_data){ .child = NULL, .data = data, .cb = cb};

    SDLNW_Widget_Recompose(widget);

    return widget;
}