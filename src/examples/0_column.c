#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include <SDL2/SDL.h>

#include "../lib/SDLNW.h"

#define WIDTH 500
#define HEIGHT 800

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("0_column.c", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDLNW_WidgetList *list = SDLNW_WidgetList_Create();

    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0xFF, 0x00, 0x00}));
    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0x00, 0xFF, 0x00}));
    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0x00, 0x00, 0xFF}));

    SDLNW_Widget* column = SDLNW_CreateColumnWidget(list);

    int running = 1;
    SDL_Event event;

    SDLNW_Widget_Size(column, &(SDL_Rect) {0, 0, WIDTH, HEIGHT});

    while(running) {
        // SDLNW_Widget_Draw(placeholder, renderer);
        // SDLNW_Widget_Draw(surface, renderer);
        SDLNW_Widget_Draw(column, renderer);

        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_Delay(50);
    }

    SDLNW_Widget_Destroy(column);

    SDL_Quit();

    return 0;
}