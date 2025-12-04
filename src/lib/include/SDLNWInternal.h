#ifndef SDLNW_INTERNAL_H
#define SDLNW_INTERNAL_H

#include "SDLNW.h"
#include <SDL2/SDL_render.h>

// init default widget
SDLNW_Widget* create_default_widget(void);

// returns 1 if a point is in a rect, else 0
bool is_point_within_rect(int x, int y, const SDL_Rect* rect);

// src_null_terminted is a null terminated array of widget pointers
// should determine its length and write it to target_length,
// allocate an array for that many widgets and copy in from src, write to target
void _sdlnw_copy_null_terminated(SDLNW_Widget** src_null_terminted, SDLNW_Widget*** target, size_t* target_length);

struct __SDLNW_TextControllerChangeListener {
    void* data;
    void (*callback) (void*,char*);
    void (*free_data)(void*);
};

// TODO think this should be public
void SDLNW_TextControllerChangeListener_Destroy(SDLNW_TextControllerChangeListener* tccl);

// memory allocation, for debugging memleaks
void* __sdlnw_debug_malloc(size_t size, const char *file, int line);
void* __sdlnw_debug_realloc(void *ptr, size_t size, const char *file, int line);
void __sdlnw_debug_free(void *ptr);

#ifdef SDLNW_DEBUG

#define __sdlnw_malloc(size) __sdlnw_debug_malloc(size, __FILE__, __LINE__)
#define __sdlnw_realloc(ptr, size) __sdlnw_debug_realloc(ptr, size, __FILE__, __LINE__)
#define __sdlnw_free(ptr) __sdlnw_debug_free(ptr)

#else

#define __sdlnw_malloc(size) malloc(size)
#define __sdlnw_realloc(ptr, size) realloc(ptr, size)
#define __sdlnw_free(ptr) free(ptr)

#endif

// a char, and were its rendered on the screen
typedef struct {
    char c;

    // TODO these should be covered by rendered_to
    int native_width;
    int native_height;

    SDL_Surface* surface;

    SDL_Rect rendered_to;
} __sdlnw_RenderedChar;

// handles the rendering and tracking of text rendered to the screen.
typedef struct {
    __sdlnw_RenderedChar* chars;
    size_t chars_len;
    size_t chars_cap;

    SDLNW_Font* font;

    int width;
    int height;
    SDL_Texture* texture;

    SDL_Colour colour;
} __sdlnw_RenderedText;

void __sdlnw_RenderedText_init(__sdlnw_RenderedText* rt, SDLNW_Font* font, SDL_Colour colour);
void __sdlnw_RenderedText_destroy(__sdlnw_RenderedText* rt);

// text has updated, respace. to width and height of 0 will be interpreted as infinity
void __sdlnw_RenderedText_set_text(__sdlnw_RenderedText* rt, const char* text, int avaliable_width);

void __sdlnw_RenderedText_render_text(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, int x, int y);
void __sdlnw_RenderedText_render_cursor(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t cursor, const SDL_Colour* colour, int xorigin, int yorigin);
void __sdlnw_RenderedText_render_highlight(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t from, size_t to, const SDL_Colour* colour, int xorigin, int yorigin);

// returns -1 (unsigned) if not found
size_t __sdlnw_RenderedText_convert_point_to_char_index(__sdlnw_RenderedText* rt, int x, int y);

// splines
typedef struct __sdlnw_int_spline_struct __sdlnw_int_spline;

__sdlnw_int_spline* __sdlnw_int_spline_create(void);
void __sdlnw_int_spline_destroy(__sdlnw_int_spline* spline);
void __sdlnw_int_spline_add_node(__sdlnw_int_spline* spline, void* data, int (*priority)(void* data, int x), int (*compute)(void* data, int x));
int __sdlnw_int_spline_compute(__sdlnw_int_spline* spline, int x);

typedef struct __sdlnw_int_spline_rounded_box_struct __sdlnw_int_spline_rounded_box;

__sdlnw_int_spline_rounded_box* __sdlnw_int_spline_rounded_box_create(int width, int height, const SDLNW_CornerRadius* radius);
void __sdlnw_int_spline_rounded_box_destroy(__sdlnw_int_spline_rounded_box* box);

bool __sdlnw_int_spline_rounded_box_within_bounds(const __sdlnw_int_spline_rounded_box* box, int x, int y);

SDL_Rect __sdlnw_add_inset(const SDL_Rect* rect, const SDLNW_Insets* inset);
SDL_Rect __sdlnw_sub_inset(const SDL_Rect* rect, const SDLNW_Insets* inset);

#define UNPACK_RECT(r) r.x, r.y, r.w, r.h

// text widgets cache renders, which only works if the renderer doesnt change
// during testing, this is expected.
void __sdlnw_clear_text_widget_cache(SDLNW_Widget* textWidget);

#endif
