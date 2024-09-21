#include "SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL_render.h>

static void draw(SDLNW_Widget* wid, SDL_Renderer* renderer) {
    // white box
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &wid->size);

    // black cross
    int x = wid->size.x;
    int y = wid->size.y;
    int w = wid->size.w;
    int h = wid->size.h;

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderDrawLine(renderer, x, y, x + w, y + h);
    SDL_RenderDrawLine(renderer, x, y + h, x + w, y);
}

SDLNW_Widget* SDLNW_CreatePlaceholderWidget() {
    SDLNW_Widget* widget = malloc(sizeof(SDLNW_Widget));

    init_default_vtable(&widget->vtable);
    widget->vtable.draw = draw;
    widget->size = (SDL_Rect){0};
    widget->data = NULL;

    widget->on_destroy_list = NULL;

    return widget;
}