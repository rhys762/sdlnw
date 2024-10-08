#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"

struct sized_box_data {
    SDLNW_Widget* child;
    SDLNW_SizedBoxWidget_Options opts;
};

static void sizedbox_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct sized_box_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void sizedbox_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct sized_box_data* data = w->data;
    w->size = *rect;

    SDLNW_Widget_Size(data->child, rect);
}

SDL_SystemCursor sizedbox_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct sized_box_data* data = w->data;

    return SDLNW_Widget_GetAppropriateCursor(data->child, x, y);
}

void sizedbox_destroy(SDLNW_Widget* w) {
    struct sized_box_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);
    data->child = NULL;

    free(data);
    w->data = NULL;
}


static SDLNW_SizeRequest sizedbox_get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
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

static void sizedbox_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct sized_box_data* data = widget->data;
    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateSizedBoxWidget(SDLNW_Widget* child, SDLNW_SizedBoxWidget_Options opts) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = sizedbox_draw;
    widget->vtable.size = sizedbox_size;
    widget->vtable.appropriate_cursor = sizedbox_appropriate_cursor;
    widget->vtable.destroy = sizedbox_destroy;
    widget->vtable.get_requested_size = sizedbox_get_requested_size;
    widget->vtable.trickle_down_event = sizedbox_trickle_down_event;

    struct sized_box_data* data = malloc(sizeof(struct sized_box_data));
    *data = (struct sized_box_data){.child = child, .opts = opts};
    widget->data = data;

    return widget;
}
