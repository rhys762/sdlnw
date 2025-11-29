#include "SDLNW.h"
#include "SDLNWInternal.h"
#include <SDL2/SDL_render.h>

// TODO memory free

struct __sdlnw_Background_struct {
    SDL_Colour colour;
    SDL_Texture* texture;

    int cached_width, cached_height;
    SDLNW_CornerRadius cached_radius;
};

static bool radius_equal(const SDLNW_CornerRadius* a, const SDLNW_CornerRadius* b) {
    return a->bottom_left == b->bottom_left ||
        a->bottom_right == b->bottom_right ||
        a->top_left == b->top_left ||
        a->top_right == b->top_right;
}

void SDLNW_RenderBackground(SDLNW_Background* bg, SDL_Renderer* renderer, const SDL_Rect* widget_net_size, const SDLNW_CornerRadius* radius) {
    // flush cache if dimensions have changed
    if (bg->texture) {
        if (bg->cached_width != widget_net_size->w || bg->cached_height != widget_net_size->h || !radius_equal(&bg->cached_radius, radius)) {
            SDL_DestroyTexture(bg->texture);
            bg->texture = NULL;
            bg->cached_width = 0;
            bg->cached_height = 0;
        }
    }

    // if cache is empty, try to render
    if (!bg->texture && widget_net_size->w && widget_net_size->h) {
        const int w = widget_net_size->w;
        const int h = widget_net_size->h;

        SDL_Texture* original_target = SDL_GetRenderTarget(renderer);
        bg->cached_width = w;
        bg->cached_height = h;

        bg->cached_radius = *radius;

        bg->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        SDL_SetRenderTarget(renderer, bg->texture);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_Colour*c = &bg->colour;
        SDL_SetRenderDrawColor(renderer, c->r, c->g, c->b, c->a);

        __sdlnw_int_spline_rounded_box* box = __sdlnw_int_spline_rounded_box_create(w, h, radius);

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                if (__sdlnw_int_spline_rounded_box_within_bounds(box, x, y)) {
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
        }

        __sdlnw_int_spline_rounded_box_destroy(box);

        SDL_SetTextureBlendMode(bg->texture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(renderer, original_target);
    }

    // copy
    if (bg->texture) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, bg->texture, NULL, widget_net_size);
    }
}

void SDLNW_DestroyBackground(SDLNW_Background* bg) {
    __sdlnw_free(bg);
}

SDLNW_Background* SDLNW_CreateSolidBackground(SDL_Colour colour) {
    SDLNW_Background* bg = __sdlnw_malloc(sizeof(SDLNW_Background));
    *bg = (SDLNW_Background){
        .colour = colour
    };
    return bg;
}
