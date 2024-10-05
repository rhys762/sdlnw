#include "SDLNW.h"
#include "internal_helpers.h"

struct sized_box_data {
    SDLNW_Widget* child;
    SDLNW_SizedBoxWidget_Options opts;
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct sized_box_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct sized_box_data* data = w->data;
    w->size = *rect;

    SDLNW_Widget_Size(data->child, rect);
}

SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct sized_box_data* data = w->data;

    return SDLNW_Widget_GetAppropriateCursor(data->child, x, y);
}

void destroy(SDLNW_Widget* w) {
    struct sized_box_data* data = w->data;
    free(data);
    w->data = NULL;
}


static SDLNW_SizeRequest get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    (void)dimension_pixels; // unused
    struct sized_box_data* data = w->data;
    SDLNW_SizeRequest req = {0};

    if (locked_dimension == SDLNW_SizingDimension_Width) {
        req.pixels = data->opts.height_pixels;
        req.shares = data->opts.height_shares;
    } else {
        req.pixels = data->opts.width_pixels;
        req.shares = data->opts.width_shares;
    }

    return req;
}

static void trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, int* allow_passthrough) {
    struct sized_box_data* data = widget->data;
    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateSizedBoxWidget(SDLNW_Widget* child, SDLNW_SizedBoxWidget_Options opts) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    widget->vtable.destroy = destroy;
    widget->vtable.get_requested_size = get_requested_size;
    widget->vtable.trickle_down_event = trickle_down_event;

    struct sized_box_data* data = malloc(sizeof(struct sized_box_data));
    *data = (struct sized_box_data){.child = child, .opts = opts};
    widget->data = data;

    return widget;
}
