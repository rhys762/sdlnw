#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"

struct composite_data {
    SDLNW_Widget* child;
    void* data;
    SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data);
};

static void composite_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct composite_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void composite_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct composite_data* data = w->data;

    w->size = *rect;

    SDLNW_Widget_Size(data->child, rect);
}

static SDL_SystemCursor composite_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct composite_data* data = w->data;
    return SDLNW_Widget_GetAppropriateCursor(data->child, x, y);
}

static void composite_destroy(SDLNW_Widget* w) {
    struct composite_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);

    __sdlnw_free(w->data);
    w->data = NULL;
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

static SDLNW_SizeResponse composite_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct composite_data* data = w->data;

    return SDLNW_Widget_GetRequestedSize(data->child, request);
}

static void composite_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct composite_data* data = widget->data;

    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateCompositeWidget(void* data, SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = composite_draw;
    widget->vtable.size = composite_size;
    widget->vtable.appropriate_cursor = composite_appropriate_cursor;
    widget->vtable.destroy = composite_destroy;
    widget->vtable.get_requested_size = composite_get_requested_size;
    widget->vtable.trickle_down_event = composite_trickle_down_event;

    widget->data = __sdlnw_malloc(sizeof(struct composite_data));
    *((struct composite_data*)widget->data) = (struct composite_data){ .child = NULL, .data = data, .cb = cb};

    SDLNW_Widget_Recompose(widget);

    return widget;
}