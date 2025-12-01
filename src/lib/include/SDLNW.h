#ifndef SDLNW_H
#define SDLNW_H

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <stdbool.h>

/*
    Public declerations for the SDLNW library.
*/

// describes rounded corners of widget
typedef struct {
    int top_left;
    int top_right;
    int bottom_left;
    int bottom_right;
} SDLNW_CornerRadius;

// describes padding or margin
typedef struct {
    int top;
    int right;
    int bottom;
    int left;
} SDLNW_Insets;

// passed to a child widget by a parent when asking
// it's required size. The child can use the
// avaliable width to determine how much height it
// needs etc
typedef struct {
    // if negative then flex
    // 0 means 0
    // in a column, height will indicate the pixels
    // that must be shared by all rows of the column
    // etc
    int total_pixels_avaliable_width;
    int total_pixels_avaliable_height;
} SDLNW_SizeRequest; // TODO poor name query

// indicates how much of avaliable space (above) is required
typedef struct {
    // non-zero indicates the widget requests a set amount of pixels
    size_t pixels;
    // non-zero indicates the widget requests as much space as possible.
    // the widget will likely be allocted a weighted portion of space
    // according to requested shares, so if this widget requests 1 share
    // and another widget 2, then this widget will recieve a third of the
    // avaliable space.
    size_t shares;
} SDLNW_DimensionSizeRequest; // TODO poor name

typedef struct {
    SDLNW_DimensionSizeRequest width;
    SDLNW_DimensionSizeRequest height;
} SDLNW_SizeResponse;

struct struct_SDLNW_Widget;
typedef struct struct_SDLNW_Widget SDLNW_Widget;

enum SDLNW_EventType {
    SDLNW_EventType_Click,
    SDLNW_EventType_MouseScroll,
    SDLNW_EventType_MouseDrag,
    SDLNW_EventType_MouseMove,
    SDLNW_EventType_KeyUp,
    SDLNW_EventType_TextInput
};

typedef struct {
    int x, y;
    Uint8 clicks;
} SDLNW_ClickEvent;

typedef struct {
    // mouse pos
    int x, y;
    // how much was scrolled, apparently some wheels can scroll in the x direction.
    int delta_x, delta_y;
} SDLNW_MouseWheelEvent;

typedef struct {
    int mouse_x, mouse_y, origin_x, origin_y;
    bool still_down;
} SDLNW_DragEvent;

typedef struct {
    int current_x, current_y;
    int last_x, last_y;
} SDLNW_MouseMotionEvent;

typedef struct {
    SDL_Keycode key;
} SDLNW_KeyUpEvent;

typedef struct {
    const char* text;
} SDLNW_TextInputEvent;

// internal v table, should not be called directly.
typedef struct {
    void (*draw_content)(void* widget_data, const SDL_Rect* content_size, SDL_Renderer* renderer);
    void (*set_content_size)(void* widget_data, const SDL_Rect* rect);

    SDL_SystemCursor (*appropriate_cursor)(SDLNW_Widget* w, int x, int y);
    void (*destroy)(SDLNW_Widget* w);
    // based on the size of a locked dimension, how big do you need for the other dimension?
    SDLNW_SizeResponse (*get_requested_size)(SDLNW_Widget* w, SDLNW_SizeRequest request);

    // user event handling
    void (*trickle_down_event)(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough);

    // a click has occurred on the app, may not be this widget
    void (*click)(SDLNW_Widget* w, SDLNW_ClickEvent* event, bool* allow_passthrough);
    void (*mouse_scroll)(SDLNW_Widget* widget, SDLNW_MouseWheelEvent* event, bool* allow_passthrough);
    void (*drag)(SDLNW_Widget* widget, SDLNW_DragEvent* event, bool* allow_passthrough);
    void (*on_hover_on)(SDLNW_Widget* widget, SDLNW_MouseMotionEvent* event, bool* allow_passthrough);
    void (*on_hover_off)(SDLNW_Widget* widget, SDLNW_MouseMotionEvent* event, bool* allow_passthrough);
    void (*on_key_up)(SDLNW_Widget* widget, SDLNW_KeyUpEvent* event, bool* allow_passthrough);
    void (*on_text_input)(SDLNW_Widget* widget, SDLNW_TextInputEvent* event, bool* allow_passthrough);
} SDLNW_WidgetVTable;

typedef struct {
    int border_width;

    SDL_Texture* cached_texture;
    int cached_width;
    int cached_height;
    SDLNW_CornerRadius cached_radius;

    void* border_data;
    void (*render)(SDL_Renderer* renderer, const SDL_Rect* widget_size, void* border_data, const SDLNW_CornerRadius* radius);
    void (*destroy_data)(void* border_data);
} SDLNW_Border;

typedef struct __SDLNW_TextControllerChangeListener SDLNW_TextControllerChangeListener;

typedef struct {
    char* text;
    size_t text_len; // includes null char
    size_t text_capacity;

    SDLNW_TextControllerChangeListener** listeners;
    size_t listeners_len;
    size_t listeners_capacity;
} SDLNW_TextController;

typedef struct __sdlnw_Background_struct SDLNW_Background;

// TODO Can this be moved to an internal .h file?
struct struct_SDLNW_Widget {
    SDLNW_WidgetVTable vtable;
    SDL_Rect net_size; // content + padding + border width + margin
    SDL_Rect content_size;
    SDL_Rect content_and_padding_size;
    SDL_Rect content_padding_and_border_size;
    // specific implementation data
    void* data;
    // on destroy data + callbacks
    void* on_destroy_list;

    SDLNW_Border* border;
    SDLNW_Insets padding;
    SDLNW_Insets margin;
    SDLNW_CornerRadius radius;
    SDLNW_Background* background;
};

typedef struct {
    int width_pixels;
    int width_shares;
    int height_pixels;
    int height_shares;
} SDLNW_SizedBoxWidgetOptions;

// this struct isn't giving anything that TTF_Font isn't
// TODO remove.
typedef struct {
    TTF_Font* font;
    int line_height;
} SDLNW_Font;

typedef struct {
    // controller for the text, MANDATORY
    // does NOT take ownership
    SDLNW_TextController* text_controller;

    // the user can highlight text, to copy from
    bool selectable;
    // if true, user can click on an edit text. Requires selectable
    bool editable;
    // allow newlines when editing
    bool allow_newlines;

    SDLNW_Font* font;
    // color to render font in
    SDL_Colour fg;
    // color to highlight selection
    SDL_Colour highlight;
} SDLNW_TextWidgetOptions;

struct struct_SDLNW_WidgetWindowSet;
typedef struct struct_SDLNW_WidgetWindowSet SDLNW_WidgetWindowSet;

// unused should be left NULL
typedef struct {
    void* data;
    void(*on_click)(void* data, int x, int y, bool* allow_passthrough);
    void(*on_mouse_hover_on)(void* data, bool* allow_passthrough);
    void(*on_mouse_hover_off)(void* data, bool* allow_passthrough);
    void(*on_key_up)(void* data, SDL_Keycode c, bool* allow_passthrough);
} SDLNW_GestureDetectorWidgetOptions;

// all optional and will be overriden by 'sensible' defaults if 0.
typedef struct {
    char* title;
    int initial_width;
    int initial_height;
    int sdl_window_flags;
} SDLNW_WidgetWindowOptions;

/*
 * Widget sizing, borders and backgrounds
 */

void SDLNW_DrawBorder(SDLNW_Border* border, SDL_Renderer* renderer, const SDL_Rect* to, const SDLNW_CornerRadius* radius);
SDLNW_Border* SDLNW_CreateSolidBorder(int width, SDL_Colour colour);
void SDLNW_DestroyBorder(SDLNW_Border* border);
void SDLNW_SetWidgetBorder(SDLNW_Widget* w, SDLNW_Border* border); // takes ownership

void SDLNW_SetWidgetCornerRadius(SDLNW_Widget* w, SDLNW_CornerRadius radius);
void SDLNW_SetWidgetPadding(SDLNW_Widget* w, SDLNW_Insets insets);
void SDLNW_SetWidgetMargin(SDLNW_Widget* w, SDLNW_Insets insets);

void SDLNW_SetWidgetBackground(SDLNW_Widget* w, SDLNW_Background* bg);
void SDLNW_RenderBackground(SDLNW_Background* bg, SDL_Renderer* renderer, const SDL_Rect* widget_net_size, const SDLNW_CornerRadius* radius);
void SDLNW_DestroyBackground(SDLNW_Background* bg);

SDLNW_Background* SDLNW_CreateSolidBackground(SDL_Colour);

/*
 * Text Controller
 */

SDLNW_TextControllerChangeListener* SDLNW_CreateTextControllerChangeListener(void* data, void (*callback) (void*,char*), void (*free_data)(void*));

/*
 * Widget interaction
 */

void SDLNW_DrawWidget(SDLNW_Widget* w, SDL_Renderer* renderer);
void SDLNW_SetWidgetNetSize(SDLNW_Widget* w, const SDL_Rect* rect);
void SDLNW_ClickWidget(SDLNW_Widget* w, int x, int y, Uint8 clicks);
void SDLNW_DragWidget(SDLNW_Widget* w, int mouse_x_start, int mouse_y_start, int mouse_x, int mouse_y, bool still_down);
SDL_SystemCursor SDLNW_GetAppropriateCursorForWidget(SDLNW_Widget* w, int x, int y);
SDLNW_SizeResponse SDLNW_GetWidgetRequestedSize(SDLNW_Widget* w, SDLNW_SizeRequest request);
void SDLNW_DestroyWidget(SDLNW_Widget* w);
void SDLNW_TrickleDownEvent(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough);
void SDLNW_MouseScrollWidget(SDLNW_Widget* w, int x, int y, int delta_x, int delta_y);
void SDLNW_MouseMotionWidget(SDLNW_Widget* w, int x, int y, int last_x, int last_y);

// adds data and callback which will be executed when the widget is destroyed
// usefull for state cleanup
void SDLNW_AddOnWidgetDestroyCb(SDLNW_Widget* w, void* data, void(*cb)(void* data));

// trigger a rebuild of a composite widget
void SDLNW_RecomposeWidget(SDLNW_Widget* w);

// router functions
void SDLNW_RouterPush(SDLNW_Widget* w, const char* path);
void SDLNW_RouterReplace(SDLNW_Widget* w, const char* path);
void SDLNW_RouterBack(SDLNW_Widget* w);
void SDLNW_RouterAddRoute(SDLNW_Widget* w, const char* path, void* data, SDLNW_Widget* build_route(void* data, const char* path));



SDLNW_Font* SDLNW_CreateFont(const char* path, int ptsize);
void SDLNW_DestroyFont(SDLNW_Font* font);

/*
    Text controller manipulation
*/

void SDLNW_InitTextController(SDLNW_TextController* tc);
void SDLNW_DestroyTextController(SDLNW_TextController* tc);

void SDLNW_TextControllerInsert(SDLNW_TextController* tc, char c, size_t idx);
void SDLNW_TextControllerRemove(SDLNW_TextController* tc, size_t idx);
void SDLNW_AddTextControllerChangeListener(SDLNW_TextController* tc, SDLNW_TextControllerChangeListener* tccl);
// returns true if successfull, false if not present
bool SDLNW_RemoveTextControllerChangeListener(SDLNW_TextController* tc, SDLNW_TextControllerChangeListener* tccl);
void SDLNW_SetTextControllerValue(SDLNW_TextController* tc, const char* text);
char* SDLNW_GetTextControllerValue(const SDLNW_TextController* tc);

/*
    Widget creators
*/

// a black cross on white background
SDLNW_Widget* SDLNW_CreatePlaceholderWidget(void);
// list is displayed top down
SDLNW_Widget* SDLNW_CreateColumnWidget(SDLNW_Widget** null_terminated_array);
// list is displayed left right
SDLNW_Widget* SDLNW_CreateRowWidget(SDLNW_Widget** null_terminated_array);
// first on bottom, last on top
SDLNW_Widget* SDLNW_CreateZStackWidget(SDLNW_Widget** null_terminated_array);
// a widget composed of other widgets, takes a builder function
SDLNW_Widget* SDLNW_CreateCompositeWidget(void* data, SDLNW_Widget*(*cb)(SDLNW_Widget* parent, void*data));
// for page routing
SDLNW_Widget* SDLNW_CreateRouterWidget(void* data, SDLNW_Widget* create_home_widget(void* data, const char* path));
// nests a widget inside a scroll area
SDLNW_Widget* SDLNW_CreateScrollWidget(SDLNW_Widget* child);
// can force a child to be a certain width/height
SDLNW_Widget* SDLNW_CreateSizedBoxWidget(SDLNW_Widget* child, SDLNW_SizedBoxWidgetOptions opts);
// detects clicks, mouse hover etc
// setting allow_passthrough to false will stop the event propogating to parents,
// ie set to true if the event should not go to widgets behind
SDLNW_Widget* SDLNW_CreateGestureDetectorWidget(SDLNW_Widget* child, SDLNW_GestureDetectorWidgetOptions options);
// centres a smaller widget within itself
SDLNW_Widget* SDLNW_CreateCentreWidget(SDLNW_Widget* child);
// draws arbritrary contents
SDLNW_Widget* SDLNW_CreateCanvasWidget(void* data, void(*cb)(void* data, const SDL_Rect* size, SDL_Renderer* renderer));
// a text widget, holds static or editable text
SDLNW_Widget* SDLNW_CreateTextWidget(SDLNW_TextWidgetOptions options);
// empty widget, for taking up space
SDLNW_Widget* SDLNW_CreateEmptyWidget(void);

/*
 * Widget Window
 */

SDLNW_WidgetWindowSet* SDLNW_CreateWidgetWindowSet(void);
void SDLNW_DestroyWidgetWindowSet(SDLNW_WidgetWindowSet* set);

// returns window id
int SDLNW_WidgetWindowSet_CreateWidgetWindow(SDLNW_WidgetWindowSet* set, SDLNW_Widget* widget, SDLNW_WidgetWindowOptions options);
void SDLNW_CreateWidgetWindowSet_step(SDLNW_WidgetWindowSet* set);
size_t SDLNW_CreateWidgetWindowSet_get_number_of_windows(SDLNW_WidgetWindowSet* set);

/*
 * Misc
 */

// return true if two corner radius' are the same
bool SDLNW_CompareCornerRadius(const SDLNW_CornerRadius* a, const SDLNW_CornerRadius* b);
// debuging mem leaks with the debug build
void SDLNW_ReportLeaks(void);

#endif
