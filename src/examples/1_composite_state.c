/*
This example shows how to load fonts and use composite widgets.
Composite widgets are comprised of other widgets and build according to a state.
That state can be adjusted as desired and the composite widget ordered to rebuild.

The app itself is a click counter, with the surface changing colour on each click.
*/

#include "SDLNW.h"
#include "SDL_stdinc.h"
#include <libgen.h>

#include <stdio.h>
#include <assert.h>

SDLNW_Font *font = NULL;

// this will track the ongoing state of our app
// the composite state will be have a reference to this
struct app_state {
    int clicked_count;
};

// the data provided to the button for its click handler
struct button_data {
    // ref to global state
    struct app_state* app_state;
    // ref to its parent, so it can trigger rebuilds
    SDLNW_Widget* composite;
};

// the onclick handler for the button
// increment the click counter and 
// order the composite widget to rebuild
void on_click(void* p, int x, int y, bool* allow_passthrough) {
    (void)x; // unused
    (void)y; // unused
    (void)allow_passthrough;

    struct button_data* data = p;
    data->app_state->clicked_count += 1;
    // UB if not given a composite widget.
    SDLNW_Widget_Recompose(data->composite);
}

// builder function for the composite widget
SDLNW_Widget* build(SDLNW_Widget* parent, void* state) {
    // since we are allocating the state, we will need to clean up later
    struct button_data* data = malloc(sizeof(struct button_data));
    data->app_state = state;
    data->composite = parent;

    // colour will oscillate
    SDLNW_Colour colour;
    if (data->app_state->clicked_count % 2) {
        colour = (SDLNW_Colour) {0xFF, 0x00, 0x00, 0xFF};
    } else {
        colour = (SDLNW_Colour) {0x00, 0xFF, 0x00, 0xFF};
    }

    SDLNW_Widget* w = NULL;

    if (data->app_state->clicked_count == 0) {
        w = SDLNW_CreateLabelWidget("Click Me!", font);
    } else {
        char* text;
        SDL_asprintf(&text, "%d clicks", data->app_state->clicked_count);

        w = SDLNW_CreateLabelWidget(text, font);

        // widgets can be told to run arbritrary cleanups on destruction,
        // here we free the text when the label is destroyed.
        SDLNW_Widget_AddOnDestroy(w, text, free);
    }

    // the zstack list, we will be putting a text label on top of a surface
    SDLNW_Widget* widgets[] = {
        SDLNW_CreateSurfaceWidget(colour),
        w,
        NULL
    };
    

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(widgets);
    SDLNW_Widget* button = SDLNW_CreateGestureDetectorWidget(zstack, (SDLNW_GestureDetectorWidget_Options){.data=data, .on_click=on_click});

    // clean up the button data that we allocated when the button is destroyed
    SDLNW_Widget_AddOnDestroy(button, data, free);

    return button;
}

int main(int argc, char** argv) {
    // no args, use relative path to find the ttf file.
    assert(argc == 1);
    const char* dir = dirname(argv[0]);
    char* fontpath = NULL;
    SDL_asprintf(&fontpath, "%s/../jbm/fonts/ttf/JetBrainsMonoNL-Regular.ttf", dir);

    // need to init ttf for font rendering
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    // 0 clicks to start with
    struct app_state state = { 0 };
    font = SDLNW_Font_Create(fontpath, 32);
    assert(font != NULL);

    // create the composite widget
    SDLNW_Widget* widget = SDLNW_CreateCompositeWidget(&state, &build);

    // defer event handling to bootstrap
    const SDLNW_BootstrapOptions options = (SDLNW_BootstrapOptions){.sdl_window_flags = SDL_WINDOW_RESIZABLE};
    SDLNW_bootstrap(widget, options);    
    // finished, clean and exit
    
    free(fontpath);
    fontpath = NULL;

    // root widget and font
    SDLNW_Widget_Destroy(widget);
    SDLNW_Font_Destroy(font);

    TTF_Quit();
    SDL_Quit();

    SDLNW_debug_report_leaks();

    return 0;
}