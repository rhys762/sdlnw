#include "../lib/SDLNW.h"

#include <stdio.h>

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
    printf("button has been clicked %d times\n", state->app_state->clicked_count);
    SDLNW_Widget_Recompose(state->composite);
}

SDLNW_Widget* build(SDLNW_Widget* parent, void* state) {
    struct composite_state* c_state = malloc(sizeof(struct composite_state));
    c_state->app_state = state;
    c_state->composite = parent;

    SDLNW_Colour colour;
    if (c_state->app_state->clicked_count % 2) {
        colour = (SDLNW_Colour) {0xFF, 0x00, 0x00};
    } else {
        colour = (SDLNW_Colour) {0x00, 0xFF, 0x00};
    }

    SDLNW_Widget* surface = SDLNW_CreateSurfaceWidget(colour);

    SDLNW_Widget* button = SDLNW_CreateButtonWidget(surface, c_state, on_click);

    // clean up state that we allocated
    SDLNW_Widget_AddOnDestroy(button, c_state, free);

    return button;
}

int main(int argc, char** argv) {
    struct app_state state = { 0 };

    SDLNW_Widget* widget = SDLNW_CreateCompositeWidget(&state, &build);

    SDLNW_bootstrap(widget);    

    SDLNW_Widget_Destroy(widget);

    return 0;
}