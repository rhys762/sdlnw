#ifndef SDLNW_H
#define SDLNW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>

/*
    Public declerations for the SDLNW library.
*/

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} SDLNW_Colour;

struct struct_SDLNW_Widget;
typedef struct struct_SDLNW_Widget SDLNW_Widget;

typedef struct {
    void (*draw)(SDLNW_Widget* w, SDL_Renderer* renderer);
    void (*size)(SDLNW_Widget* w, const SDL_Rect* rect);
    void (*click)(SDLNW_Widget* w, int x, int y);
    void (*destroy)(SDLNW_Widget* w);
} SDLNW_Widget_VTable;

// cleaner v-table calls
void SDLNW_Widget_Draw(SDLNW_Widget* w, SDL_Renderer* renderer);
void SDLNW_Widget_Size(SDLNW_Widget* w, const SDL_Rect* rect);
void SDLNW_Widget_Click(SDLNW_Widget* w, int x, int y);
void SDLNW_Widget_Destroy(SDLNW_Widget* w);
// other helpers
// adds data and callback which will be executed when the widget is destroyed
// usefull for state cleanup
void SDLNW_Widget_AddOnDestroy(SDLNW_Widget* w, void* data, void(*cb)(void* data));

// trigger a rebuild of a composite widget
void SDLNW_Widget_Recompose(SDLNW_Widget* w);

// TODO Can this be moved to an internal .h file?
struct struct_SDLNW_Widget {
    SDLNW_Widget_VTable vtable;
    SDL_Rect size;
    // specific implementation data
    void* data;
    // on destroy data + callbacks
    void* on_destroy_list;
};

/*
    For storing a group of widgets, typically for
    a row, column or zstack
*/
typedef struct {
    SDLNW_Widget** widgets;
    int len;
    int cap;
} SDLNW_WidgetList;

SDLNW_WidgetList* SDLNW_WidgetList_Create();
void SDLNW_WidgetList_Push(SDLNW_WidgetList* list, SDLNW_Widget* w);
void SDLNW_WidgetList_Destroy(SDLNW_WidgetList* list);

SDLNW_Widget* SDLNW_CreatePlaceholderWidget();
SDLNW_Widget* SDLNW_CreateSurfaceWidget(SDLNW_Colour colour);
SDLNW_Widget* SDLNW_CreateColumnWidget(SDLNW_WidgetList* list);
SDLNW_Widget* SDLNW_CreateButtonWidget(SDLNW_Widget* child, void* data, void(*cb)(void* data, int x, int y));
SDLNW_Widget* SDLNW_CreateCompositeWidget(void* data, SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data));

/*
    Bootstrap is to make a 'simple' gui app easy, in theory a main should be able to
    - create a widget
    - pass it to bootstrap.
    - destroy widget after bootstrap returns
*/

void SDLNW_bootstrap(SDLNW_Widget* widget);

#endif