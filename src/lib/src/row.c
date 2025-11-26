#include "SDLNW.h"
#include "SDLNWInternal.h"

struct row_widget_data {
    SDLNW_Widget** list;
    size_t list_len;
};

static void row_draw_content(void* d, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)content_size;
    struct row_widget_data* data = d;

    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_DrawWidget(data->list[i], renderer);
    }
}

static void row_set_content_size(void* d, const SDL_Rect* rect) {
    struct row_widget_data* data = d;

    // find out how much space we have to work with

    size_t allocated_pixels = 0;
    size_t shares = 0;

    // TOOD cache requested size? maybe
    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_SizeResponse r = SDLNW_GetWidgetRequestedSize(data->list[i], (SDLNW_SizeRequest) {.total_pixels_avaliable_height = rect->h});

        allocated_pixels += r.width.pixels;
        shares += r.width.shares;
    }

    size_t width_per_share = 0;

    if (allocated_pixels < (size_t)rect->w && shares) {
        size_t leftover = rect->w - allocated_pixels;
        width_per_share = leftover / shares;
    }

    // allocate space
    SDL_Rect sz = *rect;
    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_SizeResponse r = SDLNW_GetWidgetRequestedSize(data->list[i], (SDLNW_SizeRequest) {.total_pixels_avaliable_height = rect->h});

        int pixels = r.width.pixels + r.width.shares * width_per_share;

        sz.w = pixels;
        SDLNW_SetWidgetNetSize(data->list[i], &sz);
        sz.x += pixels;
    }
}

static SDL_SystemCursor max_cursor(SDL_SystemCursor a, SDL_SystemCursor b) {
    return (a > b) ? a : b;
}

static SDL_SystemCursor row_widget_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct row_widget_data* data = w->data;
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

static SDLNW_SizeResponse row_width_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct row_widget_data* data = w->data;

    size_t len = data->list_len;

    SDLNW_SizeResponse response = (SDLNW_SizeResponse) {0};

    for (size_t i = 0; i < len; i++) {
        SDLNW_SizeResponse r = SDLNW_GetWidgetRequestedSize(data->list[i], request);

        // width is cumulative
        response.width.pixels += r.width.pixels;
        response.width.shares += r.width.shares;

        // height is max
        response.height.pixels = max(response.height.pixels, r.height.pixels);
        response.height.shares = max(response.height.shares, r.height.shares);
    }

    return response;
}

static void row_widget_destroy(SDLNW_Widget* w) {
    struct row_widget_data* data = w->data;

    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_DestroyWidget(data->list[i]);
    }

    __sdlnw_free(data->list);
    data->list = NULL;

    data->list_len = 0;

    __sdlnw_free(data);
    w->data = NULL;
}

static void row_widget_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct row_widget_data* data = widget->data;

    for (size_t i = 0; i < data->list_len; i++) {
        SDLNW_TrickleDownEvent(data->list[i], type, event_meta, allow_passthrough);
    }
}

SDLNW_Widget* SDLNW_CreateRowWidget(SDLNW_Widget** null_terminated_array) {
    SDLNW_Widget* row_widget = create_default_widget();

    row_widget->data = __sdlnw_malloc(sizeof(struct row_widget_data));
    struct row_widget_data* data = row_widget->data;
    *data = (struct row_widget_data){0};
    _sdlnw_copy_null_terminated(null_terminated_array, &data->list, &data->list_len);

    row_widget->vtable.draw_content = row_draw_content;
    row_widget->vtable.set_content_size = row_set_content_size;
    row_widget->vtable.appropriate_cursor = row_widget_appropriate_cursor;
    row_widget->vtable.get_requested_size = row_width_get_requested_size;
    row_widget->vtable.destroy = row_widget_destroy;
    row_widget->vtable.trickle_down_event = row_widget_trickle_down_event;

    return row_widget;
}
