#include "SDLNW.h"
#include "SDLNWPrefab.h"
#include "internal_helpers.h"

void destroy_label(void* p) {
    SDLNW_TextController* tc = p;

    SDLNW_TextController_destroy(tc);
    __sdlnw_free(tc);
}

SDLNW_Widget* SDLNWPrefab_CreateLabelWidget(const char* text, SDLNW_Font* font, SDL_Colour colour) {
    SDLNW_TextController* tc = __sdlnw_malloc(sizeof(SDLNW_TextController));
    SDLNW_TextController_init(tc);
    SDLNW_TextController_set_value(tc, text);

    SDLNW_Widget* t = SDLNW_CreateTextWidget((SDLNW_TextWidgetOptions) {
        .font = font,
        .fg = colour,
        .text_controller = tc
    });
    
    SDLNW_Widget_AddOnDestroy(t, tc, destroy_label);

    return t;
}
