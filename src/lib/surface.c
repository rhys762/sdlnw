#include "SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL_render.h>

// TODO cleanup data? Valgrind didnt catch this

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    SDLNW_Colour*c = w->data;

    SDL_SetRenderDrawColor(renderer, c->r, c->g, c->b, 0xFF);
    SDL_RenderFillRect(renderer, &w->size);
}

SDLNW_Widget* SDLNW_CreateSurfaceWidget(SDLNW_Colour colour) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;

    widget->data = malloc(sizeof(SDLNW_Colour));
    *(SDLNW_Colour*)(widget->data) = colour;

    return widget;
}