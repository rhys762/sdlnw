#ifndef SDLNW_INTERNAL_HELPERS_H
#define SDLNW_INTERNAL_HELPERS_H

#include "SDLNW.h"

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

#define SDLNW_DEBUG 1
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
    SDL_Texture* texture;

    SDL_Rect rendered_to;
} __sdlnw_RenderedChar;

// handles the rendering and tracking of text rendered to the screen.
typedef struct {
    // TODO
    // bool is_editable;
    // bool is_highlightable;

    __sdlnw_RenderedChar* chars;
    size_t chars_len;
    size_t chars_cap;

    SDL_Rect position;

    SDLNW_Font* font;
} __sdlnw_RenderedText;

void __sdlnw_RenderedText_init(__sdlnw_RenderedText* rt, SDLNW_Font* font);
void __sdlnw_RenderedText_destroy(__sdlnw_RenderedText* rt);

// text has updated, respace. to width and height of 0 will be interpreted as infinity
void __sdlnw_RenderedText_set_text(__sdlnw_RenderedText* rt, const char* text, const SDL_Rect* to);

void __sdlnw_RenderedText_render_text(__sdlnw_RenderedText* rt, SDL_Renderer* renderer);
void __sdlnw_RenderedText_render_cursor(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t cursor, const SDLNW_Colour* colour);
void __sdlnw_RenderedText_render_highlight(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t from, size_t to, const SDLNW_Colour* colour);

// returns -1 (unsigned) if not found
size_t __sdlnw_RenderedText_convert_point_to_char_index(__sdlnw_RenderedText* rt, int x, int y);

#endif
