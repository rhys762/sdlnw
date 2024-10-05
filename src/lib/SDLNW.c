#include "SDLNW.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdlib.h>

void SDLNW_Widget_Draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    w->vtable.draw(w, renderer);
}

void SDLNW_Widget_Size(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->vtable.size(w, rect);
}

void SDLNW_Widget_Click(SDLNW_Widget* w, int x, int y) {
    SDLNW_Event_Click event = (SDLNW_Event_Click){
        .x = x, .y = y
    };

    SDLNW_Widget_TrickleDownEvent(w, SDLNW_EventType_Click, &event, NULL);
}

struct on_destroy_pair {
    void* data;
    void(*cb)(void* data);
};

struct on_destroy_list {
    uint cap;
    uint len;
    struct on_destroy_pair* arr;
};

void SDLNW_Widget_Destroy(SDLNW_Widget* w) {
    struct on_destroy_list* lst = w->on_destroy_list;
    if (lst != NULL) {
        for (uint i = 0; i < lst->len; i++) {
            lst->arr[i].cb(lst->arr[i].data);
        }
        free(lst->arr);
        free(lst);
        w->on_destroy_list = NULL;
    }

    w->vtable.destroy(w);
    free(w);
}

void SDLNW_Widget_AddOnDestroy(SDLNW_Widget* w, void* data, void(*cb)(void* data)) {
    struct on_destroy_list* lst = w->on_destroy_list;

    // lazy allocate
    if (lst == NULL) {
        w->on_destroy_list = malloc(sizeof(struct on_destroy_list));
        assert(w->on_destroy_list != NULL);

        lst = w->on_destroy_list;

        lst->cap = 2;
        lst->len = 0;
        lst->arr = malloc(lst->cap * sizeof(struct on_destroy_pair));
        assert(lst->arr != NULL);
    }

    // at capacity, grow
    if (lst->len == lst->cap) {
        lst->cap *= 2;
        lst->arr = realloc(lst->arr, lst->cap * sizeof(struct on_destroy_pair));
        assert(lst->arr != NULL);
    }

    lst->arr[lst->len].data = data;
    lst->arr[lst->len].cb = cb;
    lst->len += 1;
}

SDLNW_WidgetList* SDLNW_WidgetList_Create(void) {
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

SDL_SystemCursor SDLNW_Widget_GetAppropriateCursor(SDLNW_Widget* w, int x, int y) {
    return w->vtable.appropriate_cursor(w, x, y);
}

SDLNW_SizeRequest SDLNW_Widget_GetRequestedSize(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    SDLNW_SizeRequest r = w->vtable.get_requested_size(w, locked_dimension, dimension_pixels);

    // widget requested 0 pixels and 0 shares, give 1 share.
    // if (r.pixels == 0 && r.shares == 0) {
    //     r.shares = 1;
    // }

    return r;
}

void SDLNW_WidgetList_Destroy(SDLNW_WidgetList* list) {
    for (uint i = 0; i < list->len; i++) {
        SDLNW_Widget_Destroy(list->widgets[i]);
        list->widgets[i] = NULL;
    }

    free(list->widgets);
    list->widgets = NULL;
    list->cap = 0;
    list->len = 0;

    free(list);
}

void SDLNW_Widget_TrickleDownEvent(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, int* allow_passthrough) {
    int allow = 1;

    if (allow_passthrough == NULL) {
        allow_passthrough = &allow;
    }

    if (*allow_passthrough) {
        widget->vtable.trickle_down_event(widget, type, event_meta, allow_passthrough);
    }

    if (*allow_passthrough) {
        switch(type) {
            case SDLNW_EventType_Click:
                widget->vtable.click(widget, event_meta, allow_passthrough);
                break;
            case SDLNW_EventType_MouseScroll:
                widget->vtable.mouse_scroll(widget, event_meta, allow_passthrough);
                break;
        }
    }
}

void SDLNW_Widget_MouseScroll(SDLNW_Widget* w, int x, int y) {
    SDLNW_Event_MouseWheel event = (SDLNW_Event_MouseWheel){
        .x = x, .y = y
    };

    SDLNW_Widget_TrickleDownEvent(w, SDLNW_EventType_MouseScroll, &event, NULL);
}

void SDLNW_bootstrap(SDLNW_Widget* widget, SDLNW_BootstrapOptions options) {
    int running = 1;
    SDL_Event event;

    int screen_width = options.initial_width ? options.initial_width : 500;
    int screen_height = options.initial_height ? options.initial_height : 500;
    const char* title = options.title ? options.title : "SDLNW App";
    int window_flags = options.sdl_window_flags;

    SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_flags);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDLNW_Widget_Size(widget, &(SDL_Rect) {.x = 0, .y = 0, .w = screen_width, .h = screen_height});

    int mouse_x, mouse_y;

    while (running) {
        SDLNW_Widget_Draw(widget, renderer);
        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDLNW_Widget_Click(widget, mouse_x, mouse_y);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                SDL_GetMouseState(&mouse_x, &mouse_y);
                SDL_SystemCursor widget_cursor = SDLNW_Widget_GetAppropriateCursor(widget, mouse_x, mouse_y);
                SDL_Cursor* cursor =  SDL_CreateSystemCursor(widget_cursor);
                SDL_SetCursor(cursor);
            }
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    SDL_GetWindowSize(window, &screen_width, &screen_height);
                    SDLNW_Widget_Size(widget, &(SDL_Rect) {.x = 0, .y = 0, .w = screen_width, .h = screen_height});
                }
            }
            else if (event.type == SDL_MOUSEWHEEL) {
                SDLNW_Widget_MouseScroll(widget, event.wheel.x, event.wheel.y);
            }
        }

        // TODO delay? 60fps is probably fine.
        // skip delay if resize, though its probably not required.
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}