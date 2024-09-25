#include "SDLNW.h"
#include "internal_helpers.h"

struct column_data {
    SDLNW_WidgetList* list;
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct column_data* data = w->data;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Draw(data->list->widgets[i], renderer);
    }
}

// TODO, different sizing strategies.
static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct column_data* data = w->data;

    w->size = *rect;

    int height_per_widget = rect->h / data->list->len;
    SDL_Rect sz = *rect;
    sz.h = height_per_widget;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Size(data->list->widgets[i], &sz);
        sz.y += height_per_widget;
    }
}

static void click(SDLNW_Widget* w, int x, int y) {
    struct column_data* data = w->data;

    for (uint i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Click(data->list->widgets[i], x, y);
    }
}

static SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
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

static void destroy(SDLNW_Widget* w) {
    struct column_data* data = w->data;

    SDLNW_WidgetList_Destroy(data->list);
    data->list = NULL;

    free(data);
    w->data = NULL;
}

SDLNW_Widget* SDLNW_CreateColumnWidget(SDLNW_WidgetList* list) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.click = click;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    widget->vtable.destroy = destroy;

    widget->data = malloc(sizeof(struct column_data));
    *((struct column_data*)widget->data) = (struct column_data){ .list = list };

    return widget;
}