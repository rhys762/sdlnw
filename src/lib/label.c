#include "SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <assert.h>
#include <time.h>

struct label_data {
    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_Rect surface_size;
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct label_data* data = w->data;
    if (data->texture == NULL && data->surface != NULL) {
        data->texture = SDL_CreateTextureFromSurface(renderer, data->surface);
        assert(data->texture != NULL);
    }

    SDL_RenderCopy(renderer, data->texture, NULL, &data->surface_size);
}

static double min(double a, double b) {
    if (a < b) {
        return a;
    }

    return b;
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct label_data* data = w->data;
    w->size = *rect;

    // TODO center
    const int native_width = data->surface->w;
    const int native_height = data->surface->h;

    double ratio = 1.0;

    double w_ratio = (double)rect->w / (double)native_width;
    double h_ratio = (double)rect->h / (double)native_height;

    ratio = min(ratio, w_ratio);
    ratio = min(ratio, h_ratio);

    int width = ratio * (double) native_width;
    int height = ratio * (double) native_height;

    data->surface_size.w = width;
    data->surface_size.h = height;

    const int delta_width = rect->w - width;
    const int delta_height = rect->h - height;

    data->surface_size.x = rect->x + delta_width / 2;
    data->surface_size.y = rect->y + delta_height / 2;
}

static void destroy(SDLNW_Widget* w) {
    struct label_data* data = w->data;

    SDL_FreeSurface(data->surface);
    data->surface = NULL;

    if (data->texture != NULL) {
        SDL_DestroyTexture(data->texture);
        data->texture = NULL;
    } 

    free(w->data);
    w->data = NULL;
}

SDLNW_Widget* SDLNW_CreateLabelWidget(const char* text, SDLNW_Font* font) {
    SDLNW_Widget* widget = malloc(sizeof(SDLNW_Widget));

    init_default_vtable(&widget->vtable);
    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.destroy = destroy;
    widget->size = (SDL_Rect){0};

    widget->data = malloc(sizeof(struct label_data));

    widget->on_destroy_list = NULL;

    struct label_data* data = widget->data;

    // TODO
    SDL_Color fg = {0x00, 0x00, 0x00, 0xFF};
    data->surface = TTF_RenderText_Solid(font->font, text, fg);
    
    if (data->surface == NULL) {
        printf("ttf render failure: %s\n", TTF_GetError());
    }

    data->surface_size = (SDL_Rect){0};
    data->texture = NULL;

    return widget;
}