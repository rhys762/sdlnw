#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"

struct zstack_data {
    SDLNW_WidgetList* list;
};

static void zstack_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct zstack_data* data = w->data;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Draw(data->list->widgets[i], renderer);
    }
}

// TODO, different sizing strategies.
static void zstack_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct zstack_data* data = w->data;

    w->size = *rect;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Size(data->list->widgets[i], rect);
    }
}

static SDL_SystemCursor max_cursor(SDL_SystemCursor a, SDL_SystemCursor b) {
    return (a > b) ? a : b;
}

static SDL_SystemCursor zstack_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct zstack_data* data = w->data;
    SDL_SystemCursor cursor = SDL_SYSTEM_CURSOR_ARROW;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget* w = data->list->widgets[i];
        cursor = max_cursor(cursor, SDLNW_Widget_GetAppropriateCursor(w, x, y));
    }

    return cursor;
}

static void zstack_destroy(SDLNW_Widget* w) {
    struct zstack_data* data = w->data;

    SDLNW_WidgetList_Destroy(data->list);
    data->list = NULL;

    free(data);
    w->data = NULL;
}

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static SDLNW_SizeResponse zstack_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct zstack_data* data = w->data;
    SDLNW_SizeResponse response = (SDLNW_SizeResponse) {0};

    // take max
    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_SizeResponse res = SDLNW_Widget_GetRequestedSize(data->list->widgets[i], request);
        
        response.width.pixels = max(response.width.pixels, res.width.pixels);
        response.width.shares = max(response.width.shares, res.width.shares);
        response.height.pixels = max(response.height.pixels, res.height.pixels);
        response.height.shares = max(response.height.shares, res.height.shares);
    }

    return response;
}

static void zstack_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct zstack_data* data = widget->data;

    if (data->list->len == 0) {
        return;
    }

    uint i = data->list->len - 1;

    while(true) {
        SDLNW_Widget_TrickleDownEvent(data->list->widgets[i], type, event_meta, allow_passthrough);
        if (i) {
            i--;
        } else {
            break;
        }
    }
}

SDLNW_Widget* SDLNW_CreateZStackWidget(SDLNW_WidgetList* list) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = zstack_draw;
    widget->vtable.size = zstack_size;
    widget->vtable.appropriate_cursor = zstack_appropriate_cursor;
    widget->vtable.destroy = zstack_destroy;
    widget->vtable.get_requested_size = zstack_get_requested_size;
    widget->vtable.trickle_down_event = zstack_trickle_down_event;

    widget->data = malloc(sizeof(struct zstack_data));
    *((struct zstack_data*)widget->data) = (struct zstack_data){ .list = list };

    return widget;
}