#include "../include/SDLNW.h"
#include "internal_helpers.h"
#include "SDL2/SDL_ttf.h"

SDLNW_Font* SDLNW_Font_Create(const char* path, int ptsize) {
    TTF_Font* f = TTF_OpenFont(path, ptsize); 

    if (f == NULL) {
        printf("ttf failure: %s\n", TTF_GetError());
        return NULL;
    }

    SDLNW_Font* font = __sdlnw_malloc(sizeof(SDLNW_Font));

    font->font = f;
    font->line_height = TTF_FontLineSkip(f);

    return font;
}

void SDLNW_Font_Destroy(SDLNW_Font* font) {
    TTF_CloseFont(font->font);
    font->font = NULL;

    __sdlnw_free(font);
}