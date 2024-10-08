#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"
#include <SDL2/SDL_render.h>

static void placeholder_draw(SDLNW_Widget* wid, SDL_Renderer* renderer) {
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

SDLNW_Widget* SDLNW_CreatePlaceholderWidget(void) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = placeholder_draw;

    return widget;
}