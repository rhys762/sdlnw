#include "SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL_render.h>

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    SDLNW_Colour*c = w->data;

    SDL_SetRenderDrawColor(renderer, c->r, c->g, c->b, 0xFF);
    SDL_RenderFillRect(renderer, &w->size);
}

SDLNW_Widget* SDLNW_CreateSurfaceWidget(SDLNW_Colour colour) {
    SDLNW_Widget* widget = malloc(sizeof(SDLNW_Widget));

    init_default_vtable(&widget->vtable);
    widget->vtable.draw = draw;
    widget->size = (SDL_Rect){0};

    widget->data = malloc(sizeof(SDLNW_Colour));
    *(SDLNW_Colour*)(widget->data) = colour;

    widget->on_destroy_list = NULL;

    return widget;
}