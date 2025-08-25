#include "SDLNW.h"

#ifndef MISC_H
#define MISC_H

#define RED (SDL_Colour) {.r = 0xFF, .a = 0xFF}
#define WHITE (SDL_Colour) {.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF}
#define BLACK (SDL_Colour) {.a = 0xFF}
#define ORANGE (SDL_Colour) {.r = 0xFF, .g=0xA5, .a = 0xFF}
#define GOLD (SDL_Colour) {.r = 0xFF, .g=0xD7, .a = 0xFF}

extern SDLNW_Font* FONT;

typedef struct {
    bool is_done;
    char* text;
} TodoItem;

// allocate text
TodoItem TodoItem_create(bool is_done, char* text);
void TodoItem_destroy(TodoItem* item);

typedef struct {
    TodoItem* items;
    size_t items_len;
    size_t items_cap;

    SDLNW_TextController add_item_text_controller;
    SDLNW_Widget* task_display;
    SDLNW_Widget* root_widget;
} AppState;

AppState AppState_create(void);
void AppState_add_item(AppState* appstate, bool is_done, char* text);
void AppState_destroy(AppState* appstate);

#endif
