#include "SDLNW.h"
#include "internal_helpers.h"

struct zstack_data {
    SDLNW_WidgetList* list;
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct zstack_data* data = w->data;

    for (int i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Draw(data->list->widgets[i], renderer);
    }
}

// TODO, different sizing strategies.
static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct zstack_data* data = w->data;

    w->size = *rect;

    for (int i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Size(data->list->widgets[i], rect);
    }
}

static void click(SDLNW_Widget* w, int x, int y) {
    struct zstack_data* data = w->data;

    for (int i = 0; i < data->list->len; i++) {
        SDLNW_Widget_Click(data->list->widgets[i], x, y);
    }
}

static void destroy(SDLNW_Widget* w) {
    struct zstack_data* data = w->data;

    SDLNW_WidgetList_Destroy(data->list);
    data->list = NULL;

    free(data);
    w->data = NULL;
}

SDLNW_Widget* SDLNW_CreateZStackWidget(SDLNW_WidgetList* list) {
    SDLNW_Widget* widget = malloc(sizeof(SDLNW_Widget));

    init_default_vtable(&widget->vtable);
    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.click = click;
    widget->vtable.destroy = destroy;
    widget->size = (SDL_Rect){0};

    widget->data = malloc(sizeof(struct zstack_data));
    *((struct zstack_data*)widget->data) = (struct zstack_data){ .list = list };

    widget->on_destroy_list = NULL;

    return widget;
}