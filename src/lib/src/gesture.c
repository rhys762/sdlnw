#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"
#include <SDL2/SDL_mouse.h>

struct gesture_data {
    SDLNW_Widget* child;
    SDLNW_GestureDetectorWidget_Options options;
};

static void gesture_draw_content(void* d, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)content_size;

    struct gesture_data* data = d;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void gesture_set_content_size(void* d, const SDL_Rect* rect) {
    struct gesture_data* data = d;
    SDLNW_Widget_SetNetSize(data->child, rect);
}

static void gesture_click(SDLNW_Widget* w, SDLNW_Event_Click* event, bool* allow_passthrough) {
    if (!is_point_within_rect(event->x, event->y, &w->net_size)) {
        return;
    }

    struct gesture_data* data = w->data;
    if (data->options.on_click) {
        data->options.on_click(data->options.data, event->x, event->y, allow_passthrough);
    }
}

static void gesture_destroy(SDLNW_Widget* w) {
    struct gesture_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);

    __sdlnw_free(w->data);
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

static void gesture_on_key_up(SDLNW_Widget* widget, SDLNW_Event_KeyUp* event, bool* allow_passthrough) {
    (void)event; // unused
    struct gesture_data* data = widget->data;

    if (data->options.on_key_up) {
        data->options.on_key_up(data->options.data, event->key, allow_passthrough);
    }
}

SDLNW_Widget* SDLNW_CreateGestureDetectorWidget(SDLNW_Widget* child, SDLNW_GestureDetectorWidget_Options options) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = gesture_draw_content;
    widget->vtable.set_content_size = gesture_set_content_size;
    widget->vtable.click = gesture_click;
    widget->vtable.destroy = gesture_destroy;
    widget->vtable.appropriate_cursor = gesture_appropriate_cursor;
    widget->vtable.get_requested_size = gesture_get_requested_size;
    widget->vtable.trickle_down_event = gesture_trickle_down_event;
    widget->vtable.mouse_scroll = gesture_mouse_scroll;
    widget->vtable.drag = gesture_drag;
    widget->vtable.on_hover_on = gesture_on_hover_on;
    widget->vtable.on_hover_off = gesture_on_hover_off;
    widget->vtable.on_key_up = gesture_on_key_up;

    struct gesture_data* data = __sdlnw_malloc(sizeof(struct gesture_data));
    *data = (struct gesture_data){.child = child, .options = options};

    widget->data = data;

    return widget;
}
