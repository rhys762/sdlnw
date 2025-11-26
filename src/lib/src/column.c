#include "../include/SDLNW.h"
#include "../include/SDLNWInternal.h"

struct column_data {
    SDLNW_Widget** list;
    size_t list_len;
};

static void column_draw_content(void* data, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)content_size;

    struct column_data* d = data;

    for (size_t i = 0; i < d->list_len; i++) {
        SDLNW_DrawWidget(d->list[i], renderer);
    }
}

static void column_set_content_size(void* d, const SDL_Rect* rect) {
    struct column_data* data = d;

    // find out how much space we have to work with

    size_t allocated_pixels = 0;
    size_t shares = 0;

    // TOOD cache requested size? maybe
    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_SizeResponse r = SDLNW_GetWidgetRequestedSize(data->list[i], (SDLNW_SizeRequest) {.total_pixels_avaliable_width = rect->w});

        allocated_pixels += r.height.pixels;
        shares += r.height.shares;
    }

    size_t height_per_share = 0;

    if (allocated_pixels < (size_t)rect->h && shares) {
        size_t leftover = rect->h - allocated_pixels;
        height_per_share = leftover / shares;
    }

    // allocate space
    SDL_Rect sz = *rect;
    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_SizeResponse r = SDLNW_GetWidgetRequestedSize(data->list[i], (SDLNW_SizeRequest) {.total_pixels_avaliable_width = rect->w});

        int pixels = r.height.pixels + r.height.shares * height_per_share;

        sz.h = pixels;
        SDLNW_SetWidgetNetSize(data->list[i], &sz);
        sz.y += pixels;
    }
}

static SDL_SystemCursor max_cursor(SDL_SystemCursor a, SDL_SystemCursor b) {
    return (a > b) ? a : b;
}

static SDL_SystemCursor column_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct column_data* data = w->data;
    SDL_SystemCursor cursor = SDL_SYSTEM_CURSOR_ARROW;

    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_Widget* w = data->list[i];
        if (is_point_within_rect(x, y, &w->net_size)) {
            cursor = max_cursor(cursor, SDLNW_GetAppropriateCursorForWidget(w, x, y));
        }
    }

    return cursor;
}

static size_t max(size_t a, size_t b) {
    return (a > b) ? a : b;
}

static SDLNW_SizeResponse column_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct column_data* data = w->data;

    size_t len = data->list_len;

    SDLNW_SizeResponse response = (SDLNW_SizeResponse) {0};

    for (size_t i = 0; i < len; i++) {
        SDLNW_SizeResponse r = SDLNW_GetWidgetRequestedSize(data->list[i], request);

        // height is cumulative
        response.height.pixels += r.height.pixels;
        response.height.shares += r.height.shares;

        // width is max
        response.width.pixels = max(response.width.pixels, r.width.pixels);
        response.width.shares = max(response.width.shares, r.width.shares);
    }

    return response;
}

static void column_destroy(SDLNW_Widget* w) {
    struct column_data* data = w->data;

    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_DestroyWidget(data->list[i]);
    }

    __sdlnw_free(data->list);
    data->list = NULL;

    data->list_len = 0;

    __sdlnw_free(data);
    w->data = NULL;
}

static void column_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct column_data* data = widget->data;

    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_TrickleDownEvent(data->list[i], type, event_meta, allow_passthrough);
    }
}

SDLNW_Widget* SDLNW_CreateColumnWidget(SDLNW_Widget** null_terminated_array) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = column_draw_content;
    widget->vtable.set_content_size = column_set_content_size;
    widget->vtable.appropriate_cursor = column_appropriate_cursor;
    widget->vtable.destroy = column_destroy;
    widget->vtable.get_requested_size = column_get_requested_size;
    widget->vtable.trickle_down_event = column_trickle_down_event;

    widget->data = __sdlnw_malloc(sizeof(struct column_data));
    struct column_data* data = widget->data;
    *data = (struct column_data){0};
    _sdlnw_copy_null_terminated(null_terminated_array, &data->list, &data->list_len);

    return widget;
}
