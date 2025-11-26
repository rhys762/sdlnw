#include "../include/SDLNW.h"
#include "../include/SDLNWInternal.h"
#include <SDL2/SDL_render.h>

static void placeholder_draw_content(void* d, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)d;

    // white box
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, content_size);

    // black cross
    int x = content_size->x;
    int y = content_size->y;
    int w = content_size->w;
    int h = content_size->h;

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderDrawLine(renderer, x, y, x + w, y + h);
    SDL_RenderDrawLine(renderer, x, y + h, x + w, y);
}

static SDLNW_SizeResponse placeholder_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    (void)w;
    (void)request;
    return (SDLNW_SizeResponse) {
        .height = {
            .pixels = 100,
            .shares = 1
        },
        .width = {
            .pixels = 100,
            .shares = 1
        }
    };
}

SDLNW_Widget* SDLNW_CreatePlaceholderWidget(void) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = placeholder_draw_content;
    widget->vtable.get_requested_size = placeholder_get_requested_size;

    return widget;
}
