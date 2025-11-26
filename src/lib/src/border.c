#include "SDLNW.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include "SDLNWInternal.h"

struct __sdlnw_solid_border_data {
    SDL_Colour colour;
};

void SDLNW_DrawBorder(SDLNW_Border* border, SDL_Renderer* renderer, const SDL_Rect* to, const SDLNW_CornerRadius* radius) {
    struct __sdlnw_solid_border_data* d = border->border_data;

    if (to->w != border->cached_width || to->h != border->cached_height || !SDLNW_CompareCornerRadius(radius, &border->cached_radius)) {
        SDL_DestroyTexture(border->cached_texture);
        border->cached_texture = NULL;
    }

    if (border->cached_texture == NULL && to->w && to->h) {
        border->cached_width = to->w;
        border->cached_height = to->h;
        border->cached_radius = *radius;

        SDL_Texture* original_target = SDL_GetRenderTarget(renderer);
        border->cached_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, to->w, to->h);
        SDL_SetRenderTarget(renderer, border->cached_texture);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, d->colour.r, d->colour.g, d->colour.b, d->colour.a);

        __sdlnw_int_spline_rounded_box* outter = __sdlnw_int_spline_rounded_box_create(border->cached_width, border->cached_height, radius);
        __sdlnw_int_spline_rounded_box* inner = __sdlnw_int_spline_rounded_box_create(border->cached_width - 2 * border->border_width, border->cached_height - 2 * border->border_width, radius);

        for (int y = 0; y < border->cached_height; y++) {
            for (int x = 0; x < border->cached_width; x++) {
                if (!__sdlnw_int_spline_rounded_box_within_bounds(outter, x, y)) {
                    continue;
                }

                if (__sdlnw_int_spline_rounded_box_within_bounds(inner, x - border->border_width, y - border->border_width)) {
                    continue;
                }

                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        __sdlnw_int_spline_rounded_box_destroy(outter);
        __sdlnw_int_spline_rounded_box_destroy(inner);

        SDL_SetRenderTarget(renderer, original_target);
    }

    SDL_SetTextureBlendMode(border->cached_texture, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(renderer, border->cached_texture, NULL, to);
}

void border_free(void* p) {
    __sdlnw_free(p);
}

SDLNW_Border* SDLNW_CreateSolidBorder(int width, SDL_Colour colour) {
    struct __sdlnw_solid_border_data* d = __sdlnw_malloc(sizeof(struct __sdlnw_solid_border_data));
    *d = (struct __sdlnw_solid_border_data) {
        .colour = colour
    };

    SDLNW_Border* border = __sdlnw_malloc(sizeof(SDLNW_Border));
    *border = (SDLNW_Border) {
        .border_data = d,
        .destroy_data = border_free,
        .border_width = width
    };

    return border;
}

void SDLNW_DestroyBorder(SDLNW_Border* border) {
    if (border->cached_texture) {
        SDL_DestroyTexture(border->cached_texture);
        border->cached_texture = NULL;
    }

    border->destroy_data(border->border_data);
    __sdlnw_free(border);
}
