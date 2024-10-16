#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"
#include <SDL2/SDL_mouse.h>

struct gesture_data {
    SDLNW_Widget* child;
    SDLNW_GestureDetectorWidget_Options options;
};

static void gesture_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct gesture_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void gesture_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct gesture_data* data = w->data;
    w->size = *rect;
    SDLNW_Widget_Size(data->child, rect);
}

static void gesture_click(SDLNW_Widget* w, SDLNW_Event_Click* event, bool* allow_passthrough) {
    struct gesture_data* data = w->data;
    if (data->options.on_click) {
        data->options.on_click(data->options.data, event->x, event->y, allow_passthrough);
    }
}

static void gesture_destroy(SDLNW_Widget* w) {
    struct gesture_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);

    free(w->data);
    w->data = NULL;
}

static SDL_SystemCursor gesture_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct gesture_data* data = w->data;

    SDL_SystemCursor cursor = SDL_SYSTEM_CURSOR_ARROW;

    if (data->options.on_click) {
        cursor = SDL_SYSTEM_CURSOR_HAND;
    }
    else {
        cursor = SDLNW_Widget_GetAppropriateCursor(data->child, x, y);
    }

    return cursor;
}

static SDLNW_SizeResponse gesture_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct gesture_data* data = w->data;
    return SDLNW_Widget_GetRequestedSize(data->child, request);
}

static void gesture_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct gesture_data* data = widget->data;
    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

// TODO
static void gesture_mouse_scroll(SDLNW_Widget* widget, SDLNW_Event_MouseWheel* event, bool* allow_passthrough) {   
    (void)widget; // unused
    (void)event; // unused
    (void)allow_passthrough; // unused

    // struct gesture_data* data = widget->data;
}

// TODO
static void gesture_drag(SDLNW_Widget* widget, SDLNW_Event_Drag* event, bool* allow_passthrough) {
    (void)widget; // ununsed
    (void)event; // ununsed
    (void)allow_passthrough; // ununsed
    // struct gesture_data* data = widget->data;
}

static void gesture_on_hover_on(SDLNW_Widget* widget, SDLNW_Event_MouseMove* event, bool* allow_passthrough) {
    (void)event; // unused
    struct gesture_data* data = widget->data;
    
    if (data->options.on_mouse_hover_on) {
        data->options.on_mouse_hover_on(data->options.data, allow_passthrough);
    }
}

static void gesture_on_hover_off(SDLNW_Widget* widget, SDLNW_Event_MouseMove* event, bool* allow_passthrough) {
    (void)event; // unused
    struct gesture_data* data = widget->data;

    if (data->options.on_mouse_hover_off) {
        data->options.on_mouse_hover_off(data->options.data, allow_passthrough);
    }
}

SDLNW_Widget* SDLNW_CreateGestureDetectorWidget(SDLNW_Widget* child, SDLNW_GestureDetectorWidget_Options options) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = gesture_draw;
    widget->vtable.size = gesture_size;
    widget->vtable.click = gesture_click;
    widget->vtable.destroy = gesture_destroy;
    widget->vtable.appropriate_cursor = gesture_appropriate_cursor;
    widget->vtable.get_requested_size = gesture_get_requested_size;
    widget->vtable.trickle_down_event = gesture_trickle_down_event;
    widget->vtable.mouse_scroll = gesture_mouse_scroll;
    widget->vtable.drag = gesture_drag;
    widget->vtable.on_hover_on = gesture_on_hover_on;
    widget->vtable.on_hover_off = gesture_on_hover_off;

    struct gesture_data* data = malloc(sizeof(struct gesture_data));
    *data = (struct gesture_data){.child = child, .options = options};

    widget->data = data;

    return widget;
}