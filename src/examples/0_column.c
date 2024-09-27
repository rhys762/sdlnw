/*
This is a hello world example, it displays 3 surfaces in a column and otherwise does nothing.
*/

#include <SDL2/SDL.h>
#include "../lib/SDLNW.h"

#define WIDTH 500
#define HEIGHT 800

int main(void) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("0_column.c", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // widget lists are used when we pass multiple children to a parent.
    // parents will automatically clean up children widgets, we only need to
    // destroy the root widget.
    SDLNW_WidgetList *list = SDLNW_WidgetList_Create();

    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0xFF, 0x00, 0x00}));
    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0x00, 0xFF, 0x00}));
    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0x00, 0x00, 0xFF}));

    // our root widget
    SDLNW_Widget* column = SDLNW_CreateColumnWidget(list);

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