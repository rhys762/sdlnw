#include "SDLNW.h"

#include "internal_helpers.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <assert.h>

struct __sdlnw_widget_window {
    SDLNW_Widget* widget;
    SDLNW_BootstrapOptions options;

    SDL_Window* window;
    SDL_Renderer* renderer;

    int mouse_x_down, mouse_y_down, mouse_x, mouse_y;
    bool mouse_is_down;
};

struct struct_SDLNW_WidgetWindowSet {
    struct __sdlnw_widget_window* windows;
    size_t windows_len;
    size_t windows_cap;
};

static void __sdlnw_widget_window_destroy(struct __sdlnw_widget_window* w) {
    SDLNW_Widget_Destroy(w->widget);
    SDL_DestroyWindow(w->window);
    SDL_DestroyRenderer(w->renderer);
    *w = (struct __sdlnw_widget_window) {0};
}

SDLNW_WidgetWindowSet* SDLNW_CreateWidgetWindowSet(void) {
    SDLNW_WidgetWindowSet* set = __sdlnw_malloc(sizeof(SDLNW_WidgetWindowSet));

    *set = (SDLNW_WidgetWindowSet) {0};

    set->windows_cap = 2;
    set->windows = __sdlnw_malloc(sizeof(struct __sdlnw_widget_window) * set->windows_cap);

    return set;
}

void SDLNW_WidgetWindowSet_destroy(SDLNW_WidgetWindowSet* set) {

    for (size_t i = 0; i < set->windows_len; i++) {
        __sdlnw_widget_window_destroy(&set->windows[i]);
    }
    __sdlnw_free(set->windows);

    *set = (SDLNW_WidgetWindowSet) {0};
    __sdlnw_free(set);
}

void SDLNW_WidgetWindowSet_CreateWidgetWindow(SDLNW_WidgetWindowSet* set, SDLNW_Widget* widget, SDLNW_BootstrapOptions options) {
    if (set->windows_len == set->windows_cap) {
        set->windows_cap *= 2;
        set->windows = __sdlnw_realloc(set->windows, sizeof(struct __sdlnw_widget_window) * set->windows_cap);
        assert(set->windows != NULL);
    }

    struct __sdlnw_widget_window* window = &set->windows[set->windows_len++];

    *window = (struct __sdlnw_widget_window) {
        .widget = widget,
        .options = options
    };

    int screen_width = options.initial_width ? options.initial_width : 500;
    int screen_height = options.initial_height ? options.initial_height : 500;
    const char* title = options.title ? options.title : "SDLNW App";
    int window_flags = options.sdl_window_flags;

    window->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_flags);
    window->renderer = SDL_CreateRenderer(window->window, -1, 0);

    SDL_SetRenderDrawBlendMode(window->renderer, SDL_BLENDMODE_BLEND);

    SDLNW_Widget_SetNetSize(window->widget, &(SDL_Rect) {.x = 0, .y = 0, .w = screen_width, .h = screen_height});
}

static struct __sdlnw_widget_window* get_window_with_id(SDLNW_WidgetWindowSet* set, Uint32 window_id) {
    for (size_t i = 0; i < set->windows_len; i++) {
        if (SDL_GetWindowID(set->windows[i].window) == window_id) {
            return &set->windows[i];
        }
    }

    return NULL;
}

static void close_and_destroy_widget_window(SDLNW_WidgetWindowSet* set, struct __sdlnw_widget_window* w) {
    __sdlnw_widget_window_destroy(w);
    // TODO remove from array if there

    size_t i = 0;
    for (i = 0; i < set->windows_len; i++) {
        if (&set->windows[i] == w) {
            break;
        }
    }

    if (i >= set->windows_len) {
        // not found
        return;
    }

    set->windows_len--;

    for (size_t j = i; j < set->windows_len; j++) {
        set->windows[j] = set->windows[j + 1];
    }
}

static int square_sum(int a, int b) {
    return a * a + b * b;
}

void SDLNW_CreateWidgetWindowSet_step(SDLNW_WidgetWindowSet* set) {
    // draw everything.
    for (size_t i = 0; i < set->windows_len; i++) {
        struct __sdlnw_widget_window* w = &set->windows[i];

        SDLNW_Widget_Draw(w->widget, w->renderer);
        SDL_RenderPresent(w->renderer);
    }

    // handle events
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            // what do?
            // possibly nothing
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            struct __sdlnw_widget_window* window = get_window_with_id(set, event.button.windowID);

            if (window) {
                window->mouse_x_down = event.button.x;
                window->mouse_y_down = event.button.y;
                window->mouse_is_down = true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            struct __sdlnw_widget_window* window = get_window_with_id(set, event.button.windowID);

            if (window) {
                window->mouse_is_down = false;

                if (square_sum(window->mouse_x_down - window->mouse_x, window->mouse_y_down - window->mouse_y) < 25) {
                    SDLNW_Widget_Click(window->widget, window->mouse_x, window->mouse_y, event.button.clicks);
                } else {
                    SDLNW_Widget_Drag(window->widget, window->mouse_x_down, window->mouse_y_down, window->mouse_x, window->mouse_y, window->mouse_is_down);
                }
            }

        }
        else if (event.type == SDL_MOUSEMOTION) {
            struct __sdlnw_widget_window* window = get_window_with_id(set, event.motion.windowID);
            if (window) {
                int last_x = window->mouse_x;
                int last_y = window->mouse_y;

                window->mouse_x = event.motion.x;
                window->mouse_y = event.motion.y;

                if (window->mouse_is_down) {
                    SDLNW_Widget_Drag(window->widget, window->mouse_x_down, window->mouse_y_down, window->mouse_x, window->mouse_y, window->mouse_is_down);
                } else {
                    SDL_SystemCursor widget_cursor = SDLNW_Widget_GetAppropriateCursor(window->widget, window->mouse_x, window->mouse_y);
                    SDL_Cursor* cursor =  SDL_CreateSystemCursor(widget_cursor);
                    SDL_SetCursor(cursor);

                    // SDLNW_Widget_
                    SDLNW_Widget_MouseMotion(window->widget, window->mouse_x, window->mouse_y, last_x, last_y);
                }
            }
        }
        else if (event.type == SDL_WINDOWEVENT) {
            struct __sdlnw_widget_window* window = get_window_with_id(set, event.window.windowID);
            if (window) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int screen_width, screen_height;
                    SDL_GetWindowSize(window->window, &screen_width, &screen_height);
                    SDLNW_Widget_SetNetSize(window->widget, &(SDL_Rect) {.x = 0, .y = 0, .w = screen_width, .h = screen_height});
                }
                else if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
                    int last_x = window->mouse_x;
                    int last_y = window->mouse_y;

                    window->mouse_x = -1;
                    window->mouse_y = -1;

                    SDLNW_Event_MouseMove mm_event = {
                        .current_x = window->mouse_x,
                        .current_y = window->mouse_y,
                        .last_x = last_x,
                        .last_y = last_y
                    };

                    SDLNW_Widget_TrickleDownEvent(window->widget, SDLNW_EventType_MouseMove, &mm_event, NULL);
                }
                else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    close_and_destroy_widget_window(set, window);
                }
            }
        }
        else if (event.type == SDL_MOUSEWHEEL) {
            struct __sdlnw_widget_window* window = get_window_with_id(set, event.window.windowID);
            if (window) {
                SDLNW_Widget_MouseScroll(window->widget, window->mouse_x, window->mouse_y, event.wheel.x, event.wheel.y);
            }
        }
        else if (event.type == SDL_TEXTINPUT) {
            struct __sdlnw_widget_window* window = get_window_with_id(set, event.window.windowID);
            if (window) {
                SDLNW_Event_TextInput tinput = {
                    .text = event.text.text
                };

                SDLNW_Widget_TrickleDownEvent(window->widget, SDLNW_EventType_TextInput, &tinput, NULL);
            }
        }
        else if (event.type == SDL_KEYUP) {
            struct __sdlnw_widget_window* window = get_window_with_id(set, event.window.windowID);
            if (window) {
                SDLNW_Event_KeyUp ku_event = {
                    .key = event.key.keysym.sym
                };

                SDLNW_Widget_TrickleDownEvent(window->widget, SDLNW_EventType_KeyUp, &ku_event, NULL);
            }
        }
    }
}

size_t SDLNW_CreateWidgetWindowSet_get_number_of_windows(SDLNW_WidgetWindowSet* set) {
    return set->windows_len;
}
