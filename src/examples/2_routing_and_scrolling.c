/*
an example demonstrating paragraph widgets, scroll widgets and routers.
*/

#include <assert.h>
#include "../lib/SDLNW.h"
#include <libgen.h>
#include <time.h>

SDLNW_Font *font = NULL;

const char * text = "This is a paragraph widget inside a scroll widget, it should scroll if the window cannot display the whole text at once. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

SDLNW_Widget* router = NULL;

const char* paragraph_path = "paragraph";

void home_button_cb(void * data, int x, int y) {
    (void)x; // unused
    (void)y; // unused

    const char* path = data;
    SDLNW_Widget_RouterPush(router, path);
}

void route_back(void * data, int x, int y) {
    (void)data; // unused
    (void)x; // unused
    (void)y; // unused
    SDLNW_Widget_RouterBack(router);
}

SDLNW_Widget* create_home_widget(void * data, const char * path) {
    (void)data; // unused
    (void)path; // unused

    SDLNW_WidgetList* list = SDLNW_WidgetList_Create();
    SDLNW_WidgetList_Push(list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0xFF, 0x00, 0x00}));
    SDLNW_WidgetList_Push(list, SDLNW_CreateLabelWidget("Show some text", font));

    SDLNW_Widget* zstack = SDLNW_CreateZStackWidget(list);

    return SDLNW_CreateButtonWidget(zstack, (void*)paragraph_path, home_button_cb);
}

SDLNW_Widget* create_button(void) {
    SDLNW_WidgetList* zstack_list = SDLNW_WidgetList_Create();

    SDLNW_WidgetList_Push(zstack_list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {.b = 0xFF}));
    SDLNW_WidgetList_Push(zstack_list, SDLNW_CreateLabelWidget("Click to go Back", font));

    return SDLNW_CreateButtonWidget(SDLNW_CreateZStackWidget(zstack_list), NULL, route_back);
}

// a paragraph of text, and a button.
// both overlaid on a red background
SDLNW_Widget* create_paragraph(void * data, const char * path) {
    (void)data; // unused
    (void)path; // unused

    SDLNW_WidgetList* column_list = SDLNW_WidgetList_Create();

    SDLNW_WidgetList_Push(column_list, SDLNW_CreateParagraphWidget(text, font));
    SDLNW_WidgetList_Push(column_list, create_button());


    SDLNW_WidgetList* zstack_list = SDLNW_WidgetList_Create();

    SDLNW_WidgetList_Push(zstack_list, SDLNW_CreateSurfaceWidget((SDLNW_Colour) {0xFF, 0x00, 0x00}));
    SDLNW_WidgetList_Push(zstack_list, SDLNW_CreateColumnWidget(column_list));

    SDLNW_Widget* zstack =  SDLNW_CreateZStackWidget(zstack_list);
    return SDLNW_CreateScrollWidget(zstack);
}

int main(int argc, char** argv) {
    // no args, use relative path to find the ttf file.
    assert(argc == 1);
    const char* dir = dirname(argv[0]);
    char* fontpath = NULL;
    SDL_asprintf(&fontpath, "%s/jbm/fonts/ttf/JetBrainsMonoNL-Regular.ttf", dir);

    // need to init ttf for font rendering
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    font = SDLNW_Font_Create(fontpath, 32);
    assert(font != NULL);

    // setup router
    router = SDLNW_CreateRouterWidget(NULL, create_home_widget);
    SDLNW_Widget_RouterAddRoute(router, paragraph_path, NULL, create_paragraph);

    // defer event handling to bootstrap
    const SDLNW_BootstrapOptions options = (SDLNW_BootstrapOptions){.sdl_window_flags = SDL_WINDOW_RESIZABLE};
    SDLNW_bootstrap(router, options);    
    // finished, clean and exit
    
    free(fontpath);
    fontpath = NULL;

    // root widget and font
    SDLNW_Widget_Destroy(router);
    SDLNW_Font_Destroy(font);

    TTF_Quit();
    SDL_Quit();
}