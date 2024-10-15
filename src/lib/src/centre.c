#include "SDLNW.h"
#include "internal_helpers.h"

/*
    This is half implemented, will need to wait for a major refactor to SDLNW_Widget_GetRequestedSize and corresponding vtable.
*/

struct centre_data {
    SDLNW_Widget* child;  
};

static void centre_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct centre_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void centre_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct centre_data* data = w->data;

    w->size = *rect;
    SDLNW_SizeRequest sz_req = SDLNW_Widget_GetRequestedSize(data->child, SDLNW_SizingDimension_Width, rect->w);

    const int requested_height = sz_req.pixels;
    // todo
    const int requested_width = rect->w;

    // difference between us and child
    const int delta_height = rect->h - requested_height;
    const int delta_width = rect->w - requested_width;

    SDL_Rect child_sz = (SDL_Rect) {
        .x = rect->x + delta_width / 2,
        .y = rect->y + delta_height / 2,
        .h = requested_height,
        .w = requested_width
    };

    SDLNW_Widget_Size(data->child, &child_sz);
}

static SDL_SystemCursor centre_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct centre_data* data = w->data;

    return SDLNW_Widget_GetAppropriateCursor(data->child, x, y);
}

static void centre_destroy(SDLNW_Widget* w) {
    struct centre_data* data = w->data;
    SDLNW_Widget_Destroy(data->child);

    free(w->data);
    w->data = NULL;
}


static SDLNW_SizeRequest centre_get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    struct centre_data* data = w->data;
    return SDLNW_Widget_GetRequestedSize(data->child, locked_dimension, dimension_pixels);
}

static void centre_trickle_down_event(SDLNW_Widget* w, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    SDLNW_Widget_TrickleDownEvent(w, type, event_meta, allow_passthrough);
}


SDLNW_Widget* SDLNW_CreateCentreWidget(SDLNW_Widget* child) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = centre_draw;
    widget->vtable.size = centre_size;
    widget->vtable.appropriate_cursor = centre_appropriate_cursor;
    widget->vtable.destroy = centre_destroy;
    widget->vtable.get_requested_size = centre_get_requested_size;
    widget->vtable.trickle_down_event = centre_trickle_down_event;

    struct centre_data* data = malloc(sizeof(struct centre_data));
    *data= (struct centre_data){.child = child};


    return widget;
}
