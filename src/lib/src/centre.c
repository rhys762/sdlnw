#include "SDLNW.h"
#include "SDLNWInternal.h"

/*
    This is half implemented, will need to wait for a major refactor to SDLNW_GetWidgetRequestedSize and corresponding vtable.
*/

struct centre_data {
    SDLNW_Widget* child;
};

static void centre_draw_content(void* d, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)content_size;
    struct centre_data* data = d;
    SDLNW_DrawWidget(data->child, renderer);
}

static void centre_set_content_size(void* d, const SDL_Rect* rect) {
    struct centre_data* data = d;

    SDLNW_SizeResponse sz_res = SDLNW_GetWidgetRequestedSize(data->child, (SDLNW_SizeRequest){.total_pixels_avaliable_width = rect->w, .total_pixels_avaliable_height = rect->h});

    const int requested_height = sz_res.height.pixels;
    const int requested_width = sz_res.width.pixels;

    // difference between us and child
    const int delta_height = rect->h - requested_height;
    const int delta_width = rect->w - requested_width;

    SDL_Rect child_sz = (SDL_Rect) {
        .x = rect->x + delta_width / 2,
        .y = rect->y + delta_height / 2,
        .h = requested_height,
        .w = requested_width
    };

    SDLNW_SetWidgetNetSize(data->child, &child_sz);
}

static SDL_SystemCursor centre_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct centre_data* data = w->data;

    return SDLNW_GetAppropriateCursorForWidget(data->child, x, y);
}

static void centre_destroy(SDLNW_Widget* w) {
    struct centre_data* data = w->data;
    SDLNW_DestroyWidget(data->child);

    __sdlnw_free(w->data);
    w->data = NULL;
}


static SDLNW_SizeResponse centre_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct centre_data* data = w->data;
    return SDLNW_GetWidgetRequestedSize(data->child, request);
}

static void centre_trickle_down_event(SDLNW_Widget* w, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct centre_data* data = w->data;
    SDLNW_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}


SDLNW_Widget* SDLNW_CreateCentreWidget(SDLNW_Widget* child) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = centre_draw_content;
    widget->vtable.set_content_size = centre_set_content_size;
    widget->vtable.appropriate_cursor = centre_appropriate_cursor;
    widget->vtable.destroy = centre_destroy;
    widget->vtable.get_requested_size = centre_get_requested_size;
    widget->vtable.trickle_down_event = centre_trickle_down_event;

    struct centre_data* data = __sdlnw_malloc(sizeof(struct centre_data));
    *data= (struct centre_data){.child = child};
    widget->data = data;

    return widget;
}
