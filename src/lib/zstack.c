#include "SDLNW.h"
#include "internal_helpers.h"

struct zstack_data {
    SDLNW_WidgetList* list;
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct zstack_data* data = w->data;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Draw(data->list->widgets[i], renderer);
    }
}

// TODO, different sizing strategies.
static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct zstack_data* data = w->data;

    w->size = *rect;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Size(data->list->widgets[i], rect);
    }
}

static void click(SDLNW_Widget* w, int x, int y) {
    struct zstack_data* data = w->data;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Click(data->list->widgets[i], x, y);
    }
}

static SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct zstack_data* data = w->data;
    SDL_SystemCursor cursor = SDL_SYSTEM_CURSOR_ARROW;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget* w = data->list->widgets[i];
        cursor |= SDLNW_Widget_GetAppropriateCursor(w, x, y);
    }

    return cursor;
}

static void destroy(SDLNW_Widget* w) {
    struct zstack_data* data = w->data;

    SDLNW_WidgetList_Destroy(data->list);
    data->list = NULL;

    free(data);
    w->data = NULL;
}

SDLNW_Widget* SDLNW_CreateZStackWidget(SDLNW_WidgetList* list) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.click = click;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    widget->vtable.destroy = destroy;

    widget->data = malloc(sizeof(struct zstack_data));
    *((struct zstack_data*)widget->data) = (struct zstack_data){ .list = list };

    return widget;
}