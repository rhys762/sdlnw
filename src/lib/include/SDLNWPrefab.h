#ifndef SDLNWPREFAB_H
#define SDLNWPREFAB_H

#include "SDLNW.h"

SDLNW_Widget* SDLNWPrefab_CreateLabelWidget(const char* text, SDLNW_Font* font, SDL_Colour colour);

typedef struct {
    SDLNW_Font* font;
    const char* text;
    void* cb_data;
    void (*cb)(void*cb_data);

    SDL_Colour button_bg;
    SDL_Colour button_hover_bg;

    SDL_Colour text_fg;
} SDLNWPrefab_ButtonOptions;
SDLNW_Widget* SDLNWPrefab_CreateButton(SDLNWPrefab_ButtonOptions options);

#endif
