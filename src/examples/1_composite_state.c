#include "../lib/SDLNW.h"
#include "SDL_stdinc.h"

#include <stdio.h>
#include <assert.h>

SDLNW_Font *font = NULL;

struct app_state {
    int clicked_count;
};

struct composite_state {
    struct app_state* app_state;
    SDLNW_Widget* composite;
};

void on_click(void* p, int x, int y) {
    struct composite_state* state = p;
    state->app_state->clicked_count += 1;
    SDLNW_Widget_Recompose(state->composite);
}

SDLNW_Widget* build(SDLNW_Widget* parent, void* state) {
    // since we are allocating the state, we will need to clean up later
    struct composite_state* c_state = malloc(sizeof(struct composite_state));
    c_state->app_state = state;
    c_state->composite = parent;

    SDLNW_WidgetList* list = SDLNW_WidgetList_Create();

    SDLNW_Colour colour;
    if (c_state->app_state->clicked_count % 2) {
        colour = (SDLNW_Colour) {0xFF, 0x00, 0x00};
    } else {
        colour = (SDLNW_Colour) {0x00, 0xFF, 0x00};
    }

    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget(colour));

    if (c_state->app_state->clicked_count == 0) {
        SDLNW_WidgetList_Push(list, SDLNW_CreateLabelWidget("Click Me!", font));
    } else {
        char* text;
        SDL_asprintf(&text, "%d clicks", c_state->app_state->clicked_count);

        SDLNW_Widget* label = SDLNW_CreateLabelWidget(text, font);

        SDLNW_WidgetList_Push(list, label);
        // free the text when the label widget is removed
        SDLNW_Widget_AddOnDestroy(label, text, free);
    }
    

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(list);

    SDLNW_Widget* button = SDLNW_CreateButtonWidget(zstack, c_state, on_click);

    // clean up state that we allocated
    SDLNW_Widget_AddOnDestroy(button, c_state, free);

    return button;
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    struct app_state state = { 0 };
    font = SDLNW_Font_Create("./jbm/fonts/ttf/JetBrainsMonoNL-Regular.ttf", 32);
    assert(font != NULL);

    SDLNW_Widget* widget = SDLNW_CreateCompositeWidget(&state, &build);

    SDLNW_bootstrap(widget);    

    SDLNW_Widget_Destroy(widget);

    TTF_Quit();
    SDL_Quit();

    return 0;
}