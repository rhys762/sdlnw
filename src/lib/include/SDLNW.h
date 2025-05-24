#ifndef SDLNW_H
#define SDLNW_H

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <stdbool.h>
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

// indicates how much space is avaliable
typedef struct {
    // if negative then flex
    // 0 means 0
    // in a column, height will indicate the pixels
    // that must be shared by all rows of the column
    // etc
    int total_pixels_avaliable_width;
    int total_pixels_avaliable_height;
} SDLNW_SizeRequest;

// indicates how much of avaliable space (above) is required
typedef struct {
    // non-zero indicates the widget requests a set amount of pixels
    uint pixels;
    // non-zero indicates the widget requests as much space as possible.
    // the widget will likely be allocted a weighted portion of space
    // according to requested shares, so if this widget requests 1 share
    // and another widget 2, then this widget will recieve a third of the
    // avaliable space.
    uint shares;
} SDLNW_DimensionSizeRequest;

typedef struct {
    SDLNW_DimensionSizeRequest width;
    SDLNW_DimensionSizeRequest height;
} SDLNW_SizeResponse;

enum SDLNW_SizingDimension {
    SDLNW_SizingDimension_Width,
    SDLNW_SizingDimension_Height
};

enum SDLNW_EventType {
    SDLNW_EventType_Click,
    SDLNW_EventType_MouseScroll,
    SDLNW_EventType_MouseDrag, // Combine with below?
    SDLNW_EventType_MouseMove
};

typedef struct {
    int x, y;
} SDLNW_Event_Click;

typedef struct {
    // mouse pos
    int x, y;
    // how much was scrolled, apparently some wheels can scroll in the x direction.
    int delta_x, delta_y;
} SDLNW_Event_MouseWheel;

typedef struct {
    int mouse_x, mouse_y, origin_x, origin_y;
    bool still_down;
} SDLNW_Event_Drag;

typedef struct {
    int current_x, current_y;
    int last_x, last_y;
} SDLNW_Event_MouseMove;

typedef struct {
    void (*draw)(SDLNW_Widget* w, SDL_Renderer* renderer);
    void (*size)(SDLNW_Widget* w, const SDL_Rect* rect);
    SDL_SystemCursor (*appropriate_cursor)(SDLNW_Widget* w, int x, int y);
    void (*destroy)(SDLNW_Widget* w);
    // based on the size of a locked dimension, how big do you need for the other dimension?
    SDLNW_SizeResponse (*get_requested_size)(SDLNW_Widget* w, SDLNW_SizeRequest request);

    // user event handling
    void (*trickle_down_event)(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough);
    void (*click)(SDLNW_Widget* w, SDLNW_Event_Click* event, bool* allow_passthrough);
    void (*mouse_scroll)(SDLNW_Widget* widget, SDLNW_Event_MouseWheel* event, bool* allow_passthrough);
    void (*drag)(SDLNW_Widget* widget, SDLNW_Event_Drag* event, bool* allow_passthrough);
    void (*on_hover_on)(SDLNW_Widget* widget, SDLNW_Event_MouseMove* event, bool* allow_passthrough);
    void (*on_hover_off)(SDLNW_Widget* widget, SDLNW_Event_MouseMove* event, bool* allow_passthrough);
} SDLNW_Widget_VTable;

// cleaner v-table calls
void SDLNW_Widget_Draw(SDLNW_Widget* w, SDL_Renderer* renderer);
void SDLNW_Widget_Size(SDLNW_Widget* w, const SDL_Rect* rect);
void SDLNW_Widget_Click(SDLNW_Widget* w, int x, int y);
void SDLNW_Widget_Drag(SDLNW_Widget* w, int mouse_x_start, int mouse_y_start, int mouse_x, int mouse_y, bool still_down);
SDL_SystemCursor SDLNW_Widget_GetAppropriateCursor(SDLNW_Widget* w, int x, int y);
SDLNW_SizeResponse SDLNW_Widget_GetRequestedSize(SDLNW_Widget* w, SDLNW_SizeRequest request);
void SDLNW_Widget_Destroy(SDLNW_Widget* w);
void SDLNW_Widget_TrickleDownEvent(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough);
void SDLNW_Widget_MouseScroll(SDLNW_Widget* w, int x, int y, int delta_x, int delta_y);
void SDLNW_Widget_MouseMotion(SDLNW_Widget* w, int x, int y, int last_x, int last_y);
// other helpers

// adds data and callback which will be executed when the widget is destroyed
// usefull for state cleanup
void SDLNW_Widget_AddOnDestroy(SDLNW_Widget* w, void* data, void(*cb)(void* data));

// trigger a rebuild of a composite widget
void SDLNW_Widget_Recompose(SDLNW_Widget* w);

// router functions
void SDLNW_Widget_RouterPush(SDLNW_Widget* w, const char* path);
void SDLNW_Widget_RouterReplace(SDLNW_Widget* w, const char* path);
void SDLNW_Widget_RouterBack(SDLNW_Widget* w);
void SDLNW_Widget_RouterAddRoute(SDLNW_Widget* w, const char* path, void* data, SDLNW_Widget* build_route(void* data, const char* path));

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
    Font management and caching.
*/

typedef struct {
    TTF_Font* font;
} SDLNW_Font;

SDLNW_Font* SDLNW_Font_Create(const char* path, int ptsize);
void SDLNW_Font_Destroy(SDLNW_Font* font);

/*
    Widget creators
*/

// a black cross on white background
SDLNW_Widget* SDLNW_CreatePlaceholderWidget(void);
// a coloured box
SDLNW_Widget* SDLNW_CreateSurfaceWidget(SDLNW_Colour colour);
// list is displayed top down
SDLNW_Widget* SDLNW_CreateColumnWidget(SDLNW_Widget** null_terminated_array);
// first on bottom, last on top
SDLNW_Widget* SDLNW_CreateZStackWidget(SDLNW_Widget** null_terminated_array);
// a widget composed of other widgets, takes a builder function
SDLNW_Widget* SDLNW_CreateCompositeWidget(void* data, SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data));
// for page routing
SDLNW_Widget* SDLNW_CreateRouterWidget(void* data, SDLNW_Widget* create_home_widget(void* data, const char* path));
// nests a widget inside a scroll area
SDLNW_Widget* SDLNW_CreateScrollWidget(SDLNW_Widget* child);
// a label is intended for a single line of centered text.
SDLNW_Widget* SDLNW_CreateLabelWidget(const char* text, SDLNW_Font* font);
// paragraph is for longer sections of text
SDLNW_Widget* SDLNW_CreateParagraphWidget(const char* text, SDLNW_Font* font);
// forcefully sizes its child
typedef struct {
    int width_pixels;
    int width_shares;
    int height_pixels;
    int height_shares;
} SDLNW_SizedBoxWidget_Options;
// can force a child to be a certain width/height
SDLNW_Widget* SDLNW_CreateSizedBoxWidget(SDLNW_Widget* child, SDLNW_SizedBoxWidget_Options opts);
// detects clicks, mouse hover etc
// unused should be left NULL
// setting allow_passthrough to false will stop the event propogating to parents,
// ie set to true if the event should not go to widgets behind
typedef struct {
    void* data;
    void(*on_click)(void* data, int x, int y, bool* allow_passthrough);
    void(*on_mouse_hover_on)(void* data, bool* allow_passthrough);
    void(*on_mouse_hover_off)(void* data, bool* allow_passthrough);
} SDLNW_GestureDetectorWidget_Options;
SDLNW_Widget* SDLNW_CreateGestureDetectorWidget(SDLNW_Widget* child, SDLNW_GestureDetectorWidget_Options options);
// centres a smaller widget within itself
SDLNW_Widget* SDLNW_CreateCentreWidget(SDLNW_Widget* child);
// draws arbritrary contents
SDLNW_Widget* SDLNW_CreateCanvasWidget(void* data, void(*cb)(void* data, const SDL_Rect* size, SDL_Renderer* renderer));

// all optional and will be overriden by 'sensible' defaults if 0.
typedef struct {
    char* title;
    int initial_width;
    int initial_height;
    int sdl_window_flags;
} SDLNW_BootstrapOptions;

/*
    Bootstrap is to make a 'simple' gui app easy, in theory a main should be able to
    - init sdl and ttf
    - create a widget
    - pass it to bootstrap.
    - destroy widget after bootstrap returns
    - quit sdl and ttf
*/

void SDLNW_bootstrap(SDLNW_Widget* widget, SDLNW_BootstrapOptions options);

#endif
