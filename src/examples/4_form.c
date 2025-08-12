#include <stdio.h>
#include <assert.h>
#include <libgen.h>
#include "SDLNW.h"

struct app_data {
    SDLNW_TextController tc;
    SDLNW_Font* font;
};

static void listener(void* data, char* text) {
    (void)data;

    printf("text is now '%s'\n", text);
}

SDLNW_Widget* app_builder(SDLNW_Widget* parent, void* d) {
    (void)parent;
    struct app_data* data = d;

    SDLNW_TextWidgetOptions toptions = {
        .text_controller = &data->tc,
        .font = data->font,
        .fg = (SDLNW_Colour) { 0x00, 0x00, 0x00, 0xFF},
        .highlight = (SDLNW_Colour) {0x00, 0x00, 0x00, 0x55},
        .editable = true,
        .selectable = true
    };

    SDLNW_Widget* label = SDLNW_CreateTextWidget(toptions);

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget((SDLNW_Widget*[]) {
        SDLNW_CreateSurfaceWidget((SDLNW_Colour){.r = 0xFF, .g=0xFF, .b=0xFF, .a=0xFF}),
        label,
        NULL
    });

    return zstack;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const char* dir = dirname(argv[0]);

    char* fontpath = NULL;
    SDL_asprintf(&fontpath, "%s/../jbm/fonts/ttf/JetBrainsMonoNL-Regular.ttf", dir);

    // need to init ttf for font rendering
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    SDL_StartTextInput();

    SDLNW_Font* font = SDLNW_Font_Create(fontpath, 22);
    assert(font != NULL);

    SDLNW_Widget* widget;
    struct app_data data = {
        .font = font
    };

    SDLNW_TextController_init(&data.tc);
    SDLNW_TextControllerChangeListener* tccl =  SDLNW_CreateTextControllerChangeListener(NULL, listener, NULL);
    SDLNW_TextController_add_change_listener(&data.tc, tccl);

    widget = SDLNW_CreateCompositeWidget(&data, app_builder);

    SDLNW_bootstrap(widget, (SDLNW_BootstrapOptions) {
        .title = "form"
    });

    SDLNW_Widget_Destroy(widget);
    SDLNW_Font_Destroy(font);
    SDLNW_TextController_destroy(&data.tc);

    SDL_StopTextInput();
    TTF_Quit();
    SDL_Quit();

    SDLNW_debug_report_leaks();
}