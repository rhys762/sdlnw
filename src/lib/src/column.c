#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"

struct column_data {
    SDLNW_WidgetList* list;
};

static void column_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct column_data* data = w->data;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Draw(data->list->widgets[i], renderer);
    }
}

static void column_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct column_data* data = w->data;

    w->size = *rect;

    // find out how much space we have to work with

    uint allocated_pixels = 0;
    uint shares = 0;

    // TOOD cache requested size? maybe
    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_SizeRequest r = SDLNW_Widget_GetRequestedSize(data->list->widgets[i], SDLNW_SizingDimension_Width, rect->w);

        allocated_pixels += r.pixels;
        shares += r.shares;
    }

    uint height_per_share = 0;

    if (allocated_pixels < (uint)rect->h && shares) {
        uint leftover = rect->h - allocated_pixels;
        height_per_share = leftover / shares;
    }

    // allocate space
    SDL_Rect sz = *rect;
    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_SizeRequest r = SDLNW_Widget_GetRequestedSize(data->list->widgets[i], SDLNW_SizingDimension_Width, rect->w);

        int pixels = r.pixels + r.shares * height_per_share;

        sz.h = pixels;
        SDLNW_Widget_Size(data->list->widgets[i], &sz);
        sz.y += pixels;
    }
}

static SDL_SystemCursor column_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct column_data* data = w->data;
    SDL_SystemCursor cursor = SDL_SYSTEM_CURSOR_ARROW;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget* w = data->list->widgets[i];
        if (is_point_within_rect(x, y, &w->size)) {
            cursor |= SDLNW_Widget_GetAppropriateCursor(w, x, y);
        }
    }

    return cursor;
}

static uint max(uint a, uint b) {
    return (a > b) ? a : b;
}

static SDLNW_SizeRequest column_get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    struct column_data* data = w->data;

    uint len = data->list->len;

    uint requested_pixels = 0;
    uint requested_shares = 0;

    for (uint i = 0; i < len; i++) {
        SDLNW_SizeRequest r = SDLNW_Widget_GetRequestedSize(data->list->widgets[i], locked_dimension, dimension_pixels);


        if (locked_dimension == SDLNW_SizingDimension_Height) {
            // max
            requested_pixels = max(r.pixels, requested_pixels);
            requested_shares = max(r.shares, requested_shares);
        } else {
            // cumulative
            requested_pixels += r.pixels;
            requested_shares += r.shares;
        }
    }

    SDLNW_SizeRequest req = (SDLNW_SizeRequest){0};

    req.shares = requested_shares;
    req.pixels = requested_pixels;

    return req;
}

static void column_destroy(SDLNW_Widget* w) {
    struct column_data* data = w->data;

    SDLNW_WidgetList_Destroy(data->list);
    data->list = NULL;

    free(data);
    w->data = NULL;
}

static void column_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct column_data* data = widget->data;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_TrickleDownEvent(data->list->widgets[i], type, event_meta, allow_passthrough);
    }
}

SDLNW_Widget* SDLNW_CreateColumnWidget(SDLNW_WidgetList* list) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = column_draw;
    widget->vtable.size = column_size;
    widget->vtable.appropriate_cursor = column_appropriate_cursor;
    widget->vtable.destroy = column_destroy;
    widget->vtable.get_requested_size = column_get_requested_size;
    widget->vtable.trickle_down_event = column_trickle_down_event;

    widget->data = malloc(sizeof(struct column_data));
    *((struct column_data*)widget->data) = (struct column_data){ .list = list };

    return widget;
}
