#include "SDLNW.h"
#include "SDLNWInternal.h"
#include <SDL2/SDL_render.h>

struct canvas_data {
    void* data;
    void(*cb)(void* data, const SDL_Rect* size, SDL_Renderer* renderer);
};

static void canvas_draw_content(void* data, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    struct canvas_data* d = data;
    d->cb(d->data, content_size, renderer);
}

SDLNW_Widget* SDLNW_CreateCanvasWidget(void* data, void(*cb)(void* data, const SDL_Rect* size, SDL_Renderer* renderer)) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = canvas_draw_content;

    struct canvas_data* d = __sdlnw_malloc(sizeof(struct canvas_data));
    *d = (struct canvas_data){
        .data = data,
        .cb = cb
    };
    widget->data = d;

    return widget;
}
