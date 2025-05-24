/*
This is a hello world example, it displays 3 surfaces in a column and otherwise does nothing.
*/

#include <SDL2/SDL.h>
#include "SDLNW.h"

#define WIDTH 500
#define HEIGHT 800

int main(void) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("0_column.c", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // some widgets accept child widgets as null terminated arrays.
    // the parent will immediately copy those children into its own internal
    // buffer, taking ownership of the child widgets but NOT the array itself
    SDLNW_Widget* widgets[] = {
        SDLNW_CreateSizedBoxWidget(SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0xFF, 0x00, 0x00}), (SDLNW_SizedBoxWidget_Options){.height_shares = 1}),
        SDLNW_CreateSizedBoxWidget(SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0x00, 0xFF, 0x00}), (SDLNW_SizedBoxWidget_Options){.height_shares = 1}),
        SDLNW_CreateSizedBoxWidget(SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0x00, 0x00, 0xFF}), (SDLNW_SizedBoxWidget_Options){.height_shares = 1}),
        NULL
    };

    // our root widget
    SDLNW_Widget* column = SDLNW_CreateColumnWidget(widgets);

    int running = 1;
    SDL_Event event;

    // tell the widget where it is located, in all of our examples this will be the entirety of the window
    // but this is not a requirement.
    SDLNW_Widget_Size(column, &(SDL_Rect) {0, 0, WIDTH, HEIGHT});

    // manual event loop handling, simple apps can consider void SDLNW_bootstrap(SDLNW_Widget* widget) instead which
    // is used in other examples.
    while(running) {
        SDLNW_Widget_Draw(column, renderer);

        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_Delay(50);
    }

    // clean up our root widget
    SDLNW_Widget_Destroy(column);

    SDL_Quit();

    return 0;
}