#include "SDLNW.h"
#include "src/lib/internal_helpers.h"
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
    uint routes_len;
    uint routes_cap;

    SDLNW_Widget** stack;
    uint stack_len;
    uint stack_cap;

    SDLNW_Widget* current;
    SDLNW_Widget* buffer;
};

static void resolve_buffer(struct router_data* data) {
    if (data->buffer == NULL) {
        return;
    }

    SDLNW_Widget_Destroy(data->current);
    data->current = data->buffer;
    data->buffer = NULL;
}

static SDLNW_Widget* find_widget(struct router_data* data, const char* path) {
    SDLNW_Widget* w = NULL;

    for (uint i = 0; i < data->routes_len; i++) {
        if (0 == strcmp(path, data->routes[i].path)) {
            w = data->routes[i].build_route(data->routes[i].d, path);
            break;
        }
    }

    return w;
}

void SDLNW_Widget_RouterPush(SDLNW_Widget* w, const char* path) {
    struct router_data* data = w->data;

    if (data->stack_cap == data->stack_len) {
        data->stack_cap *= 2;
        data->stack = realloc(data->stack, data->stack_cap * sizeof(char*));
        assert(data->stack != NULL);
    }

    SDLNW_Widget* new_widget = find_widget(data, path);
    if (new_widget == NULL) {
        new_widget = SDLNW_CreatePlaceholderWidget();
    }

    if (data->stack_len == data->stack_cap) {
        data->stack_cap *= 2;
        data->stack = realloc(data->stack, data->stack_cap);
        assert(data->stack != NULL);
    }

    data->stack[data->stack_len] = new_widget;
    data->stack_len += 1;

    data->current = new_widget;
    SDLNW_Widget_Size(data->current, &w->size);
}

void SDLNW_Widget_RouterReplace(SDLNW_Widget* w, const char* path) {
    struct router_data* data = w->data;

    if (data->stack_len == 0) {
        SDLNW_Widget_RouterPush(w, path);
        return;
    }

    SDLNW_Widget* new_widget = find_widget(data, path);
    if (new_widget == NULL) {
        new_widget = SDLNW_CreatePlaceholderWidget();
    }

    data->stack[data->stack_len - 1] = new_widget;
    data->buffer = new_widget;
    SDLNW_Widget_Size(data->buffer, &w->size);
}

void SDLNW_Widget_RouterBack(SDLNW_Widget* w) {
    struct router_data* data = w->data;

    if (data->stack_len == 0) {
        return;
    }

    data->stack_len -= 1;

    if (data->stack_len) {
        data->buffer = data->stack[data->stack_len - 1];
    } else {
        // home
        data->buffer = find_widget(data, "");
    }
    SDLNW_Widget_Size(data->buffer, &w->size);
}

void SDLNW_Widget_RouterAddRoute(SDLNW_Widget* w, const char* path, void* d, SDLNW_Widget* build_route(void* data, const char* path)) {
    struct router_data* data = w->data;

    if (data->routes_len == data->routes_cap) {
        data->routes_cap *= 2;
        data->routes = realloc(data->routes, data->routes_cap * sizeof(struct route));
        assert(data->routes != NULL);
    }

    data->routes[data->routes_len] = (struct route){path, d, build_route};
    data->routes_len += 1;
}

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct router_data* data = w->data;
    resolve_buffer(data);
    SDLNW_Widget_Draw(data->current, renderer);
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct router_data* data = w->data;

    w->size = *rect;

    resolve_buffer(data);
    SDLNW_Widget_Size(data->current, rect);
}

static SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct router_data* data = w->data;
    
    resolve_buffer(data);
    return SDLNW_Widget_GetAppropriateCursor(data->current, x, y);
}

static void destroy(SDLNW_Widget* w) {
    struct router_data* data = w->data;

    resolve_buffer(data);

    free(data->routes);

    for (uint i = 0; i < data->stack_len; i++) {
        SDLNW_Widget_Destroy(data->stack[i]);
        data->stack[i] = NULL;
    }

    free(data->stack);

    free(data);
    w->data = NULL;
}

static SDLNW_SizeRequest get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    struct router_data* data = w->data;

    return SDLNW_Widget_GetRequestedSize(data->current, locked_dimension, dimension_pixels);
}

static void trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, int* allow_passthrough) {
    struct router_data* data = widget->data;

    SDLNW_Widget_TrickleDownEvent(data->current, type, event_meta, allow_passthrough);
}

SDLNW_Widget* SDLNW_CreateRouterWidget(void* data, SDLNW_Widget* create_home_widget(void* data, const char* path)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    widget->vtable.destroy = destroy;
    widget->vtable.get_requested_size = get_requested_size;
    widget->vtable.trickle_down_event = trickle_down_event;

    widget->data = malloc(sizeof(struct router_data));
    struct router_data* d = widget->data;
    *d = (struct router_data){0};

    d->routes_cap = 2;
    d->routes = malloc(d->routes_cap * sizeof(struct route));
    assert(d->routes != NULL);

    d->stack_cap = 2;
    d->stack = malloc(d->stack_cap * sizeof(char*));
    assert(d->stack != NULL);

    SDLNW_Widget_RouterAddRoute(widget, "", data, create_home_widget);

    d->current = find_widget(d, "");
    SDLNW_Widget_Size(d->current, &widget->size);

    return widget;
}
