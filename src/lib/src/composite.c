#include "../include/SDLNW.h"
#include "../include/SDLNWInternal.h"

struct composite_data {
    SDLNW_Widget* child;
    // widget we will be replacing child with
    SDLNW_Widget* buffer;

    void* data;
    SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data);
};

static void resolve_buffer(struct composite_data* data, const SDL_Rect* content_size) {
    if (data->buffer) {
        if (data->child) {
            SDLNW_DestroyWidget(data->child);
        }
        data->child = data->buffer;
        data->buffer = NULL;

        SDLNW_SetWidgetNetSize(data->child, content_size);
    }
}

static void composite_draw_content(void* data, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)content_size;

    struct composite_data* d = data;
    resolve_buffer(data, content_size);
    SDLNW_DrawWidget(d->child, renderer);
}

static void composite_set_content_size(void* data, const SDL_Rect* rect) {
    struct composite_data* d = data;

    SDLNW_SetWidgetNetSize(d->child, rect);
}

static SDL_SystemCursor composite_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct composite_data* data = w->data;
    return SDLNW_GetAppropriateCursorForWidget(data->child, x, y);
}

static void composite_destroy(SDLNW_Widget* w) {
    struct composite_data* data = w->data;

    SDLNW_DestroyWidget(data->child);

    __sdlnw_free(w->data);
    w->data = NULL;
}

void SDLNW_RecomposeWidget(SDLNW_Widget* w) {
    struct composite_data* data = w->data;

    if (data->buffer != NULL) {
        SDLNW_DestroyWidget(data->buffer);
        data->buffer = NULL;
    }

    data->buffer = data->cb(w, data->data);
}

static SDLNW_SizeResponse composite_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct composite_data* data = w->data;

    return SDLNW_GetWidgetRequestedSize(data->child, request);
}

static void composite_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct composite_data* data = widget->data;

    SDLNW_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateCompositeWidget(void* data, SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = composite_draw_content;
    widget->vtable.set_content_size = composite_set_content_size;
    widget->vtable.appropriate_cursor = composite_appropriate_cursor;
    widget->vtable.destroy = composite_destroy;
    widget->vtable.get_requested_size = composite_get_requested_size;
    widget->vtable.trickle_down_event = composite_trickle_down_event;

    widget->data = __sdlnw_malloc(sizeof(struct composite_data));
    *((struct composite_data*)widget->data) = (struct composite_data){ .child = NULL, .data = data, .cb = cb};

    SDLNW_RecomposeWidget(widget);
    resolve_buffer(widget->data, &widget->content_size);

    return widget;
}
