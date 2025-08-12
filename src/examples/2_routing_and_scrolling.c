/*
an example demonstrating paragraph widgets, scroll widgets and routers.

Also demonstrates hover for gesture widgets, used to alter button colour on hover.
*/

#include <assert.h>
#include "SDLNW.h"
#include <libgen.h>
#include <time.h>

SDLNW_Font *font = NULL;

SDLNW_Widget* router = NULL;

const char* paragraph_path = "paragraph";

/*
    The buttons heirachy is, top down:
    1: the gesture widget (uses clicks to route, hover to update state, will tell composite(2) to rebuild)
    2: the composite widget (rebuilds contents when told to)
    3: a zstack widget with the following bottom up:
        1: a surface widget, for the background
        2: a label widget presenting the text

    When the button is clicked we send a command to the router.
    When the button is hovered, the button state is modified and the composite told to rebuild.
*/

struct button_data {
    bool hovered;
    SDLNW_Colour colour;
    const char* text;
    SDLNW_Widget* comp;
};

void button_on_mouse_hover_on(void* data, bool* allow_passthrough) {
    struct button_data* button_data = data;

    button_data->hovered = true;
    *allow_passthrough = false;
    SDLNW_Widget_Recompose(button_data->comp);
}

void button_on_mouse_hover_off(void* data, bool* allow_passthrough) {
    struct button_data* button_data = data;

    button_data->hovered = false;
    *allow_passthrough = false;
    SDLNW_Widget_Recompose(button_data->comp);
}

static Uint8 increment_without_overflow(Uint8 a, Uint8 b) {
    Uint16 aa = a;
    Uint16 bb = b;

    Uint16 cc = aa + bb;

    if (cc > 0xFF) {
        cc = 0xFF;
    }

    return cc;
}

// the builder given to the composite widget
SDLNW_Widget* create_button_builder(SDLNW_Widget* parent, void* data) {
    (void)parent; // not used, rebuilding is handled by the gesture widget

    struct button_data* button_data = data;

    SDLNW_Colour c = button_data->colour;

    // add brightness to button on hover.
    if (button_data->hovered) {
        c = (SDLNW_Colour){
            .r = increment_without_overflow(c.r, 30),
            .g = increment_without_overflow(c.g, 30),
            .b = increment_without_overflow(c.b, 30),
            .a = c.a
        };
    }

    SDLNW_Widget* widgets[] = {
        SDLNW_CreateSurfaceWidget(c),
        SDLNW_CreateLabelWidget(button_data->text, font),
        NULL
    };

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(widgets);

    return zstack;
}

// creates a generic button that changes colour on hover
SDLNW_Widget* create_button(const char* text, SDLNW_Colour colour, void (*cb)(void* data, int x, int y, bool* allow_passthrough)) {
    struct button_data* data = malloc(sizeof(struct button_data));
    *data = (struct button_data) {.colour = colour, .text = text};

    SDLNW_Widget* comp = SDLNW_CreateCompositeWidget(data, create_button_builder);
    data->comp = comp;

    SDLNW_Widget* gesture = SDLNW_CreateGestureDetectorWidget(comp, (SDLNW_GestureDetectorWidget_Options){
        .data = data,
        .on_click = cb,
        .on_mouse_hover_on = button_on_mouse_hover_on,
        .on_mouse_hover_off = button_on_mouse_hover_off
    });

    SDLNW_Widget_AddOnDestroy(gesture, data, free);

    return gesture;
}

// button cb for the home page button, takes us to the paragraph page
void home_button_cb(void * data, int x, int y, bool* allow_passthrough) {
    (void)data; // unused
    (void)x; // unused
    (void)y; // unused
    (void)allow_passthrough; // unused
    
    SDLNW_Widget_RouterPush(router, paragraph_path);
}

// button cb for the paragraph page, will pop the paragraph page off the stack, deleting it
// and take the user back to the home page
void route_back(void * data, int x, int y, bool* allow_passthrough) {
    (void)data; // unused
    (void)x; // unused
    (void)y; // unused
    (void)allow_passthrough; // unused

    SDLNW_Widget_RouterBack(router);
}

SDLNW_Widget* create_home_widget(void * data, const char * path) {
    (void)data; // unused
    (void)path; // unused

    SDLNW_Colour red = (SDLNW_Colour) {.r=0xFF, .a=0XFF};
    return create_button("Show some text", red, home_button_cb);
}

// a paragraph of text, and a button.
// both overlaid on a red background
SDLNW_Widget* create_paragraph(void * data, const char * path) {
    (void)path; // unused
    SDLNW_TextController* tc = data;

    SDLNW_Colour blue = {.b = 0xFF, .a=0XFF};

    SDLNW_Widget* text = SDLNW_CreateTextWidget((SDLNW_TextWidgetOptions) {
        .allow_newlines = true,
        .fg = (SDLNW_Colour) {0x00, 0x00, 0x00, 0xFF},
        .highlight = (SDLNW_Colour) {0x00, 0x00, 0x00, 0x99},
        .font = font,
        .selectable = true,
        .text_controller = tc
    });

    SDLNW_Widget* column_widgets[] = {
        text,
        create_button("Click to go Back", blue, route_back),
        NULL
    };

    SDLNW_Widget* zstack_widgets[] = {
        SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0xFF, 0x00, 0x00, 0xFF}),
        SDLNW_CreateColumnWidget(column_widgets),
        NULL
    };

    SDLNW_Widget* zstack =  SDLNW_CreateZStackWidget(zstack_widgets);
    return SDLNW_CreateScrollWidget(zstack);
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

    SDLNW_TextController tc;
    SDLNW_TextController_init(&tc);
    SDLNW_TextController_set_value(&tc, "This is a paragraph widget inside a scroll widget, it should scroll if the window cannot display the whole text at once. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

    font = SDLNW_Font_Create(fontpath, 32);
    assert(font != NULL);

    // setup router
    router = SDLNW_CreateRouterWidget(NULL, create_home_widget);
    SDLNW_Widget_RouterAddRoute(router, paragraph_path, &tc, create_paragraph);

    // defer event handling to bootstrap
    const SDLNW_BootstrapOptions options = (SDLNW_BootstrapOptions){.sdl_window_flags = SDL_WINDOW_RESIZABLE};
    SDLNW_bootstrap(router, options);    
    // finished, clean and exit
    
    free(fontpath);
    fontpath = NULL;

    // root widget and font
    SDLNW_Widget_Destroy(router);
    SDLNW_Font_Destroy(font);

    SDLNW_TextController_destroy(&tc);

    TTF_Quit();
    SDL_Quit();

    SDLNW_debug_report_leaks();
}
