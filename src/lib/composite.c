#include "SDLNW.h"
#include "internal_helpers.h"

struct composite_data {
    SDLNW_Widget* child;
    void* data;
    SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data);
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct composite_data* data = w->data;
    SDLNW_Widget_Draw(data->child, renderer);
}

static void click(SDLNW_Widget* w, int x, int y) {
    struct composite_data* data = w->data;
    SDLNW_Widget_Click(data->child, x, y);
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct composite_data* data = w->data;

    w->size = *rect;

    SDLNW_Widget_Size(data->child, rect);
}

static void destroy(SDLNW_Widget* w) {
    struct composite_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);
}

void SDLNW_Widget_Recompose(SDLNW_Widget* w) {
    struct composite_data* data = w->data;

    if (data->child != NULL) {
        SDLNW_Widget_Destroy(data->child);
        data->child = NULL;
    }
    
    data->child = data->cb(w, data->data);
    SDLNW_Widget_Size(data->child, &w->size);
}

SDLNW_Widget* SDLNW_CreateCompositeWidget(void* data, SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data)) {
    SDLNW_Widget* widget = malloc(sizeof(SDLNW_Widget));

    init_default_vtable(&widget->vtable);

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.click = click;
    widget->vtable.destroy = destroy;
    widget->size = (SDL_Rect){0};

    widget->data = malloc(sizeof(struct composite_data));
    *((struct composite_data*)widget->data) = (struct composite_data){ .child = NULL, .data = data, .cb = cb};

    widget->on_destroy_list = NULL;

    SDLNW_Widget_Recompose(widget);

    return widget;
}