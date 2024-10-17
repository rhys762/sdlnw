/*
    Demonstrates canvas, centre and sized.

    Presents a square spinning arround the centre of the window
*/

#include "SDLNW.h"
#include "SDL_render.h"
#include "SDL_timer.h"
#include <libgen.h>
#include <assert.h>
#include <math.h>

void draw_square_at(SDL_Renderer* renderer, int x, int y) {
    int width = 30;
    int half_width = width / 2;

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_Rect square = {x - half_width, y - half_width, width, width};
    SDL_RenderFillRect(renderer, &square);
}

void canvas_draw(void* data, const SDL_Rect* size, SDL_Renderer* renderer) {
    (void)data; // unused

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, size);

    Uint32 t = SDL_GetTicks();

    int centre_of_size_x = size->x + size->w / 2;
    int centre_of_size_y = size->y + size->h / 2;

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderDrawPoint(renderer, centre_of_size_x, centre_of_size_y);

    int radius = 100;

    double theta = (double)t / 1000.0;

    draw_square_at(renderer, centre_of_size_x + radius * cos(theta), centre_of_size_y + radius * sin(theta));
}

SDLNW_Widget* create_app(void) {
    SDLNW_WidgetList* list = SDLNW_WidgetList_Create();

    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour){.b = 0xFF}));
    SDLNW_WidgetList_Push(list, 
    SDLNW_CreateCentreWidget(
        SDLNW_CreateSizedBoxWidget(
            SDLNW_CreateCanvasWidget(NULL, canvas_draw),
        (SDLNW_SizedBoxWidget_Options){.width_pixels = 200, .height_pixels = 200})
        )
    );

    return SDLNW_CreateZStackWidget(list);
}

int main(void) {
    // need to init ttf for font rendering
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    // create the composite widget
    SDLNW_Widget* widget = create_app();

    // defer event handling to bootstrap
    const SDLNW_BootstrapOptions options = (SDLNW_BootstrapOptions){.sdl_window_flags = SDL_WINDOW_RESIZABLE};
    SDLNW_bootstrap(widget, options);    

    // root widget
    SDLNW_Widget_Destroy(widget);

    TTF_Quit();
    SDL_Quit();

    return 0;
}

