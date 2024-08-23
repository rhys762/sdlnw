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

    SDL_Window* window = SDL_CreateWindow("window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDLNW_Widget* placeholder = SDLNW_CreatePlaceholderWidget();
    SDLNW_Widget_Size(placeholder, &(SDL_Rect) {0, 0, WIDTH, HEIGHT});

    SDLNW_Widget* surface = SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0xFF, 0x00, 0x00});
    SDLNW_Widget_Size(surface, &(SDL_Rect) {0, 0, WIDTH, HEIGHT});

    int running = 1;
    SDL_Event event;

    while(running) {
        // SDLNW_Widget_Draw(placeholder, renderer);
        SDLNW_Widget_Draw(surface, renderer);

        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_Delay(500);
    }

    SDL_Quit();

    return 0;
}