#include "../include/SDLNW.h"
#include "../include/SDLNWInternal.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct route {
    const char* path;
    void* d;
    SDLNW_Widget* (*build_route)(void* data, const char* path);
};

struct router_data {
    struct route* routes;
    size_t routes_len;
    size_t routes_cap;

    SDLNW_Widget** stack;
    size_t stack_len;
    size_t stack_cap;

    SDLNW_Widget* current;
    SDLNW_Widget* buffer;
};

static void resolve_buffer(struct router_data* data) {
    if (data->buffer == NULL) {
        return;
    }

    SDLNW_DestroyWidget(data->current);
    data->current = data->buffer;
    data->buffer = NULL;
}

static SDLNW_Widget* find_widget(struct router_data* data, const char* path) {
    SDLNW_Widget* w = NULL;

    for (size_t i = 0; i < data->routes_len; i++) {
        if (0 == strcmp(path, data->routes[i].path)) {
            w = data->routes[i].build_route(data->routes[i].d, path);
            break;
        }
    }

    return w;
}

void SDLNW_RouterPush(SDLNW_Widget* w, const char* path) {
    struct router_data* data = w->data;

    if (data->stack_cap == data->stack_len) {
        data->stack_cap *= 2;
        data->stack = __sdlnw_realloc(data->stack, data->stack_cap * sizeof(char*));
        assert(data->stack != NULL);
    }

    SDLNW_Widget* new_widget = find_widget(data, path);
    if (new_widget == NULL) {
        new_widget = SDLNW_CreatePlaceholderWidget();
    }

    if (data->stack_len == data->stack_cap) {
        data->stack_cap *= 2;
        data->stack = __sdlnw_realloc(data->stack, data->stack_cap);
        assert(data->stack != NULL);
    }

    data->stack[data->stack_len] = new_widget;
    data->stack_len += 1;

    data->current = new_widget;
    SDLNW_SetWidgetNetSize(data->current, &w->content_size);
}

void SDLNW_RouterReplace(SDLNW_Widget* w, const char* path) {
    struct router_data* data = w->data;

    if (data->stack_len == 0) {
        SDLNW_RouterPush(w, path);
        return;
    }

    SDLNW_Widget* new_widget = find_widget(data, path);
    if (new_widget == NULL) {
        new_widget = SDLNW_CreatePlaceholderWidget();
    }

    data->stack[data->stack_len - 1] = new_widget;
    data->buffer = new_widget;
    SDLNW_SetWidgetNetSize(data->buffer, &w->content_size);
}

void SDLNW_RouterBack(SDLNW_Widget* w) {
    struct router_data* data = w->data;

    if (data->stack_len <= 1) {
        return;
    }

    data->stack_len -= 1;

    if (data->stack_len) {
        data->buffer = data->stack[data->stack_len - 1];
    } else {
        // home
        data->buffer = find_widget(data, "");
    }
    SDLNW_SetWidgetNetSize(data->buffer, &w->content_size);
}

void SDLNW_RouterAddRoute(SDLNW_Widget* w, const char* path, void* d, SDLNW_Widget* build_route(void* data, const char* path)) {
    struct router_data* data = w->data;

    if (data->routes_len == data->routes_cap) {
        data->routes_cap *= 2;
        data->routes = __sdlnw_realloc(data->routes, data->routes_cap * sizeof(struct route));
        assert(data->routes != NULL);
    }

    data->routes[data->routes_len] = (struct route){path, d, build_route};
    data->routes_len += 1;
}

static void router_draw_content(void* d, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)content_size;
    struct router_data* data = d;
    resolve_buffer(data);
    SDLNW_DrawWidget(data->current, renderer);
}

static void router_set_content_size(void* d, const SDL_Rect* rect) {
    struct router_data* data = d;

    resolve_buffer(data);
    SDLNW_SetWidgetNetSize(data->current, rect);
}

static SDL_SystemCursor router_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct router_data* data = w->data;

    resolve_buffer(data);
    return SDLNW_GetAppropriateCursorForWidget(data->current, x, y);
}

static void router_destroy(SDLNW_Widget* w) {
    struct router_data* data = w->data;

    resolve_buffer(data);

    __sdlnw_free(data->routes);

    for (size_t i = 0; i < data->stack_len; i++) {
        SDLNW_DestroyWidget(data->stack[i]);
        data->stack[i] = NULL;
    }

    __sdlnw_free(data->stack);

    __sdlnw_free(data);
    w->data = NULL;
}

static SDLNW_SizeResponse router_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    struct router_data* data = w->data;

    return SDLNW_GetWidgetRequestedSize(data->current, request);
}

static void router_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct router_data* data = widget->data;

    SDLNW_TrickleDownEvent(data->current, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateRouterWidget(void* data, SDLNW_Widget* create_home_widget(void* data, const char* path)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = router_draw_content;
    widget->vtable.set_content_size = router_set_content_size;
    widget->vtable.appropriate_cursor = router_appropriate_cursor;
    widget->vtable.destroy = router_destroy;
    widget->vtable.get_requested_size = router_get_requested_size;
    widget->vtable.trickle_down_event = router_trickle_down_event;

    widget->data = __sdlnw_malloc(sizeof(struct router_data));
    struct router_data* d = widget->data;
    *d = (struct router_data){0};

    d->routes_cap = 2;
    d->routes = __sdlnw_malloc(d->routes_cap * sizeof(struct route));
    assert(d->routes != NULL);

    d->stack_cap = 2;
    d->stack = __sdlnw_malloc(d->stack_cap * sizeof(char*));
    assert(d->stack != NULL);

    SDLNW_RouterAddRoute(widget, "", data, create_home_widget);

    SDLNW_RouterPush(widget, "");
    SDLNW_SetWidgetNetSize(d->current, &widget->content_size);

    return widget;
}
