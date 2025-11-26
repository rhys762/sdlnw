#include "test_misc.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include "SDL_surface.h"
#include "SDL_image.h"
#include <float.h>

typedef struct {
    int r, g, b;
} ColourAverage;

static ColourAverage averageFor(const SDL_Surface* surface, int x, int y, int w, int h) {
    ColourAverage avg = {0};
    int pixel_size = surface->format->BytesPerPixel;
    Uint8* pixels = surface->pixels;

    Uint8 r, g, b;

    if ((x + w) > surface->w) {
        w = surface->w - x;
    }

    if ((y + h) > surface->h) {
        h = surface->h - y;
    }

    for (int xx = 0; xx < w; xx++) {
        for (int yy = 0; yy < h; yy++) {
            int pixel_x = x + xx;
            int pixel_y = y + yy;

            Uint8* pixel = pixels + pixel_size * pixel_x + surface->pitch * pixel_y;


            SDL_GetRGB(*(Uint32*)pixel, surface->format, &r, &g, &b);
            avg.r += r;
            avg.g += g;
            avg.b += b;
        }
    }

    int number_of_pixels = w * h;

    avg.r /= number_of_pixels;
    avg.g /= number_of_pixels;
    avg.b /= number_of_pixels;

    return avg;
}

static ColourAverage absDiff(const ColourAverage* a, const ColourAverage* b) {
    ColourAverage avg = {
        .r = a->r - b->r,
        .g = a->g - b->g,
        .b = a->b - b->b
    };

    if (avg.r < 0) {
        avg.r *= -1;
    }

    if (avg.g < 0) {
        avg.g *= -1;
    }

    if (avg.b < 0) {
        avg.b *= -1;
    }

    return avg;
}

// divide images into a grid of squares, avergage r,g,b accross square
// take abs diff between squares in the same place and return sum
// divided by number of pixels
static double compare_surfaces(const SDL_Surface* a, const SDL_Surface* b) {
    if (a->w != b->w || a->h != b->h) {
        // dims should match, else test should probably be updated anyway
        return DBL_MAX;
    }

    int square_width = 30;
    int acc = 0;

    for (int x = 0; x < a->w; x += square_width) {
        for (int y = 0; y < a->h; y += square_width) {
            ColourAverage avg_a = averageFor(a, x, y, square_width, square_width);
            ColourAverage avg_b = averageFor(b, x, y, square_width, square_width);
            ColourAverage avg_diff = absDiff(&avg_a, &avg_b);
            acc += (avg_diff.r + avg_diff.g + avg_diff.b);
        }
    }

    return (double)acc / (double)(a->w * a->h);
}

double compareImages(const char* save_as, const char* test_against, SDLNW_Widget* widget, const SDL_Rect* size) {
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_CreateWindowAndRenderer(100, 100, SDL_WINDOW_HIDDEN, &window, &renderer);

    if (size != NULL) {
        SDLNW_SetWidgetNetSize(widget, size);
    }

    SDL_Texture* renderedWidget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, widget->net_size.w, widget->net_size.h);
    SDL_SetRenderTarget(renderer, renderedWidget);

    // render the widget

    SDLNW_DrawWidget(widget, renderer);

    SDL_Surface* renderedWidgetSurface = createSurfaceFromTexture(renderer, renderedWidget);

    // save to file so test can be updated if required
    IMG_SavePNG(renderedWidgetSurface, save_as);

    // retreive test case
    SDL_Surface* test_case = IMG_Load(test_against);

    double out = DBL_MAX;

    if (test_case != NULL) {
        out = compare_surfaces(renderedWidgetSurface, test_case);
    }

    SDL_FreeSurface(test_case);
    SDL_FreeSurface(renderedWidgetSurface);
    SDL_DestroyTexture(renderedWidget);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return out;
}

SDL_Surface* createSurfaceFromTexture(SDL_Renderer* renderer, SDL_Texture* texture) {
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA8888);

    if (!surface) {
        return NULL;
    }

    SDL_Texture* target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);

    if (SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch) < 0) {
        fprintf(stderr, "SDL_RenderReadPixels failed: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        surface = NULL;
    }

    SDL_SetRenderTarget(renderer, target);

    return surface;
}

void dummy_render(SDLNW_Widget* w) {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_CreateWindowAndRenderer(100, 100, SDL_WINDOW_HIDDEN, &window, &renderer);

    SDLNW_DrawWidget(w, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
