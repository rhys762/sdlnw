#include "SDLNW.h"
#include <assert.h>
#include <stdlib.h>

void SDLNW_Widget_Draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    w->vtable.draw(w, renderer);
}

void SDLNW_Widget_Size(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->vtable.size(w, rect);
}

void SDLNW_Widget_Destroy(SDLNW_Widget* w) {
    w->vtable.destroy(w);
    free(w);
}

SDLNW_WidgetList* SDLNW_WidgetList_Create() {
    SDLNW_WidgetList* list = malloc(sizeof(SDLNW_WidgetList));

    list->cap = 4;
    list->len = 0;
    list->widgets = malloc(list->cap * sizeof(SDLNW_Widget*));

    return list;
}

void SDLNW_WidgetList_Push(SDLNW_WidgetList* list, SDLNW_Widget* w) {
    if (list->len >= list->cap) {
        list->cap *= 2;
        list->widgets = realloc(list->widgets, list->cap * sizeof(SDLNW_Widget*));
        assert(list->widgets != NULL);
    }

    list->widgets[list->len] = w;
    list->len += 1;
}

void SDLNW_WidgetList_Destroy(SDLNW_WidgetList* list) {
    for (int i = 0; i < list->len; i++) {
        SDLNW_Widget_Destroy(list->widgets[i]);
        list->widgets[i] = NULL;
    }

    free(list->widgets);
    list->widgets = NULL;
    list->cap = 0;
    list->len = 0;

    free(list);
}