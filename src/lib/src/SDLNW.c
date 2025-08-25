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

bool SDLNW_CornerRadius_comp(const SDLNW_CornerRadius* a, const SDLNW_CornerRadius* b) {
    return a->bottom_left == b->bottom_left &&
        a->bottom_right == b->bottom_right &&
        a->top_left == b->top_left &&
        a->top_right == b->top_right;
}

void SDLNW_Widget_Draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    if (w->background) {
        SDLNW_Background_render(w->background, renderer, &w->net_size, &w->radius);
    }

    if (w->border) {
        SDLNW_Border_draw(w->border, renderer, &w->net_size, &w->radius);
    }

    w->vtable.draw_content(w->data, &w->content_size, renderer);
}

void SDLNW_Widget_SetNetSize(SDLNW_Widget* w, const SDL_Rect* rect) {
    w->net_size = *rect;
    w->content_size = __sdlnw_sub_inset(rect, &w->padding);


    w->vtable.set_content_size(w->data, &w->content_size);
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
    if (w->border) {
        SDLNW_Border_destroy(w->border);
    }

    if (w->background) {
        SDLNW_Background_destroy(w->background);
    }

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
    int padding_width = w->padding.left + w->padding.right;
    int padding_height = w->padding.top + w->padding.bottom;

    // subtract off padding
    if (request.total_pixels_avaliable_width) {
        request.total_pixels_avaliable_width -= padding_width;
    }
    if (request.total_pixels_avaliable_height) {
        request.total_pixels_avaliable_height -= padding_height;
    }

    SDLNW_SizeResponse r = w->vtable.get_requested_size(w, request);

    r.width.pixels += padding_width;
    r.height.pixels += padding_height;

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
                    if (is_point_within_rect(e->x, e->y, &widget->net_size)) {
                        widget->vtable.mouse_scroll(widget, e, allow_passthrough);
                    }
                }

                break;
            case SDLNW_EventType_MouseDrag:
                {
                    SDLNW_Event_Drag* e = event_meta;
                    if (is_point_within_rect(e->mouse_x, e->mouse_y, &widget->net_size) || is_point_within_rect(e->origin_x, e->origin_y, &widget->net_size)) {
                        widget->vtable.drag(widget, event_meta, allow_passthrough);
                    }
                }
                break;
            case SDLNW_EventType_MouseMove:
                {
                    SDLNW_Event_MouseMove* e = event_meta;

                    bool is_within = is_point_within_rect(e->current_x, e->current_y, &widget->net_size);
                    bool was_within = is_point_within_rect(e->last_x, e->last_y, &widget->net_size);

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

void SDLNW_Widget_add_border(SDLNW_Widget* w, SDLNW_Border* border) {
    if (w->border) {
        SDLNW_Border_destroy(w->border);
    }

    w->border = border;
}

void SDLNW_Widget_set_corner_radius(SDLNW_Widget* w, SDLNW_CornerRadius radius) {
    w->radius = radius;
}

void SDLNW_Widget_set_padding(SDLNW_Widget* w, SDLNW_Insets insets) {
    w->padding = insets;
}
