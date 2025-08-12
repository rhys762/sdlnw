#include "../include/SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void SDLNW_Widget_Draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    w->vtable.draw(w, renderer);
}

void SDLNW_Widget_Size(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->vtable.size(w, rect);
}

void SDLNW_Widget_Click(SDLNW_Widget* w, int x, int y, Uint8 clicks) {
    SDLNW_Event_Click event = (SDLNW_Event_Click){
        .x = x, .y = y, .clicks = clicks
    };

    SDLNW_Widget_TrickleDownEvent(w, SDLNW_EventType_Click, &event, NULL);
}

void SDLNW_Widget_Drag(SDLNW_Widget* w, int mouse_x_start, int mouse_y_start, int mouse_x, int mouse_y, bool still_down) {
    SDLNW_Event_Drag event = {.origin_x = mouse_x_start, .origin_y = mouse_y_start, .mouse_x = mouse_x, .mouse_y = mouse_y, .still_down = still_down};

    SDLNW_Widget_TrickleDownEvent(w, SDLNW_EventType_MouseDrag, &event, NULL);
}

struct on_destroy_pair {
    void* data;
    void(*cb)(void* data);
};

struct on_destroy_list {
    size_t cap;
    size_t len;
    struct on_destroy_pair* arr;
};

void SDLNW_Widget_Destroy(SDLNW_Widget* w) {
    struct on_destroy_list* lst = w->on_destroy_list;
    if (lst != NULL) {
        for (size_t i = 0; i < lst->len; i++) {
            lst->arr[i].cb(lst->arr[i].data);
        }
        __sdlnw_free(lst->arr);
        __sdlnw_free(lst);
        w->on_destroy_list = NULL;
    }

    w->vtable.destroy(w);
    __sdlnw_free(w);
}

void SDLNW_Widget_AddOnDestroy(SDLNW_Widget* w, void* data, void(*cb)(void* data)) {
    struct on_destroy_list* lst = w->on_destroy_list;

    // lazy allocate
    if (lst == NULL) {
        w->on_destroy_list = __sdlnw_malloc(sizeof(struct on_destroy_list));
        assert(w->on_destroy_list != NULL);

        lst = w->on_destroy_list;

        lst->cap = 2;
        lst->len = 0;
        lst->arr = __sdlnw_malloc(lst->cap * sizeof(struct on_destroy_pair));
        assert(lst->arr != NULL);
    }

    // at capacity, grow
    if (lst->len == lst->cap) {
        lst->cap *= 2;
        lst->arr = __sdlnw_realloc(lst->arr, lst->cap * sizeof(struct on_destroy_pair));
        assert(lst->arr != NULL);
    }

    lst->arr[lst->len].data = data;
    lst->arr[lst->len].cb = cb;
    lst->len += 1;
}

SDL_SystemCursor SDLNW_Widget_GetAppropriateCursor(SDLNW_Widget* w, int x, int y) {
    return w->vtable.appropriate_cursor(w, x, y);
}

SDLNW_SizeResponse SDLNW_Widget_GetRequestedSize(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    SDLNW_SizeResponse r = w->vtable.get_requested_size(w, request);

    return r;
}

void SDLNW_Widget_TrickleDownEvent(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    bool allow = true;

    if (allow_passthrough == NULL) {
        allow_passthrough = &allow;
    }

    if (*allow_passthrough) {
        widget->vtable.trickle_down_event(widget, type, event_meta, allow_passthrough);
    }

    if (*allow_passthrough) {
        switch(type) {
            case SDLNW_EventType_Click:
                {
                    SDLNW_Event_Click* e = event_meta;
                    widget->vtable.click(widget, e, allow_passthrough);
                }
                break;
            case SDLNW_EventType_MouseScroll:
                {
                    SDLNW_Event_MouseWheel* e = event_meta;
                    if (is_point_within_rect(e->x, e->y, &widget->size)) {
                        widget->vtable.mouse_scroll(widget, e, allow_passthrough);
                    }
                }
                
                break;
            case SDLNW_EventType_MouseDrag:
                {
                    SDLNW_Event_Drag* e = event_meta;
                    if (is_point_within_rect(e->mouse_x, e->mouse_y, &widget->size) || is_point_within_rect(e->origin_x, e->origin_y, &widget->size)) {
                        widget->vtable.drag(widget, event_meta, allow_passthrough);
                    }
                }
                break;
            case SDLNW_EventType_MouseMove:
                {
                    SDLNW_Event_MouseMove* e = event_meta;

                    bool is_within = is_point_within_rect(e->current_x, e->current_y, &widget->size);
                    bool was_within = is_point_within_rect(e->last_x, e->last_y, &widget->size);

                    if (is_within && !was_within) {
                        widget->vtable.on_hover_on(widget, e, allow_passthrough);
                    } else if (!is_within && was_within) {
                        widget->vtable.on_hover_off(widget, e, allow_passthrough);
                    }
                }
                break;
            case SDLNW_EventType_KeyUp:
                {
                    SDLNW_Event_KeyUp* e = event_meta;
                    widget->vtable.on_key_up(widget, e, allow_passthrough);
                }
                break;
            case SDLNW_EventType_TextInput:
                {
                    SDLNW_Event_TextInput*e = event_meta;
                    widget->vtable.on_text_input(widget, e, allow_passthrough);
                }
        }
    }
}

void SDLNW_Widget_MouseScroll(SDLNW_Widget* w, int x, int y, int delta_x, int delta_y) {
    SDLNW_Event_MouseWheel event = (SDLNW_Event_MouseWheel){
        .x = x, .y = y, .delta_x = delta_x, .delta_y = delta_y
    };

    SDLNW_Widget_TrickleDownEvent(w, SDLNW_EventType_MouseScroll, &event, NULL);
}

void SDLNW_Widget_MouseMotion(SDLNW_Widget* w, int x, int y, int last_x, int last_y) {
    SDLNW_Event_MouseMove event = (SDLNW_Event_MouseMove) {
        .current_x = x,
        .current_y = y,
        .last_x = last_x,
        .last_y = last_y
    };

    SDLNW_Widget_TrickleDownEvent(w, SDLNW_EventType_MouseMove, &event, NULL);
}

static int square_sum(int a, int b) {
    return a * a + b * b;
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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDLNW_Widget_Size(widget, &(SDL_Rect) {.x = 0, .y = 0, .w = screen_width, .h = screen_height});

    int mouse_x_down, mouse_y_down, mouse_x, mouse_y;
    bool mouse_is_down = false;

    while (running) {
        SDLNW_Widget_Draw(widget, renderer);
        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                mouse_x_down = event.button.x;
                mouse_y_down = event.button.y;
                mouse_is_down = true;
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                mouse_x = event.button.x;
                mouse_y = event.button.y;
                mouse_is_down = false;

                if (square_sum(mouse_x_down - mouse_x, mouse_y_down - mouse_y) < 25) {
                    SDLNW_Widget_Click(widget, mouse_x, mouse_y, event.button.clicks);
                } else {
                    SDLNW_Widget_Drag(widget, mouse_x_down, mouse_y_down, mouse_x, mouse_y, mouse_is_down);
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int last_x = mouse_x;
                int last_y = mouse_y;

                mouse_x = event.motion.x;
                mouse_y = event.motion.y;

                if (mouse_is_down) {
                    SDLNW_Widget_Drag(widget, mouse_x_down, mouse_y_down, mouse_x, mouse_y, mouse_is_down);
                } else {
                    SDL_SystemCursor widget_cursor = SDLNW_Widget_GetAppropriateCursor(widget, mouse_x, mouse_y);
                    SDL_Cursor* cursor =  SDL_CreateSystemCursor(widget_cursor);
                    SDL_SetCursor(cursor);

                    // SDLNW_Widget_
                    SDLNW_Widget_MouseMotion(widget, mouse_x, mouse_y, last_x, last_y);
                }                
            }
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    SDL_GetWindowSize(window, &screen_width, &screen_height);
                    SDLNW_Widget_Size(widget, &(SDL_Rect) {.x = 0, .y = 0, .w = screen_width, .h = screen_height});
                }
                else if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
                    int last_x = mouse_x;
                    int last_y = mouse_y;

                    mouse_x = -1;
                    mouse_y = -1;

                    SDLNW_Event_MouseMove mm_event = {
                        .current_x = mouse_x,
                        .current_y = mouse_y,
                        .last_x = last_x,
                        .last_y = last_y
                    };

                    SDLNW_Widget_TrickleDownEvent(widget, SDLNW_EventType_MouseMove, &mm_event, NULL);
                }
            }
            else if (event.type == SDL_MOUSEWHEEL) {
                SDLNW_Widget_MouseScroll(widget, mouse_x, mouse_y, event.wheel.x, event.wheel.y);
            }
            else if (event.type == SDL_TEXTINPUT) {
                SDLNW_Event_TextInput tinput = {
                    .text = event.text.text
                };

                SDLNW_Widget_TrickleDownEvent(widget, SDLNW_EventType_TextInput, &tinput, NULL);
            }
            else if (event.type == SDL_KEYUP) {
                SDLNW_Event_KeyUp ku_event = {
                    .key = event.key.keysym.sym
                };

                SDLNW_Widget_TrickleDownEvent(widget, SDLNW_EventType_KeyUp, &ku_event, NULL);
            }
        }

        // TODO delay? 60fps is probably fine.
        // skip delay if resize, though its probably not required.
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}