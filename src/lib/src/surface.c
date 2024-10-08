#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"
#include <SDL2/SDL_render.h>

static void surface_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    SDLNW_Colour*c = w->data;

    SDL_SetRenderDrawColor(renderer, c->r, c->g, c->b, 0xFF);
    SDL_RenderFillRect(renderer, &w->size);
}

static void surface_destroy(SDLNW_Widget* w) {
    free(w->data);
    w->data = NULL;
}

SDLNW_Widget* SDLNW_CreateSurfaceWidget(SDLNW_Colour colour) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = surface_draw;
    widget->vtable.destroy = surface_destroy;

    widget->data = malloc(sizeof(SDLNW_Colour));
    *(SDLNW_Colour*)(widget->data) = colour;

    return widget;
}