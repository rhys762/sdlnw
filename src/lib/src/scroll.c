#include "../include/SDLNW.h"
#include "../include/SDLNWInternal.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <assert.h>

struct scroll_data {
    SDLNW_Widget* child;

    SDL_Texture* texture;
    int texture_width, texture_height;

    SDL_Rect window;
    int y_window_max;

    SDL_Rect y_scroll_bar;
    bool dragging_y_scroll;
    int start_scroll_drag_x, start_scroll_drag_y;
};

static void scroll_draw_content(void* d, const SDL_Rect* content_size, SDL_Renderer* renderer) {
    (void)content_size;
    struct scroll_data* data = d;

    if (data->texture == NULL) {
        data->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, data->texture_width, data->texture_height);
        SDL_SetTextureBlendMode(data->texture, SDL_BLENDMODE_BLEND);
    }

    SDL_Texture* original_target = SDL_GetRenderTarget(renderer);

    SDL_SetRenderTarget(renderer, data->texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderClear(renderer);

    SDLNW_DrawWidget(data->child, renderer);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, original_target);
    SDL_RenderCopy(renderer, data->texture, &data->window, content_size);

    // y scroll bar
    if (data->y_scroll_bar.h) {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xAA);
        SDL_BlendMode mode;
        SDL_GetRenderDrawBlendMode(renderer, &mode);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &data->y_scroll_bar);
        SDL_SetRenderDrawBlendMode(renderer, mode);
    }
}

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static void scroll_set_content_size(void* d, const SDL_Rect* rect) {
    struct scroll_data* data = d;

    SDLNW_SizeResponse response = SDLNW_GetWidgetRequestedSize(data->child, (SDLNW_SizeRequest) {.total_pixels_avaliable_width = rect->w, .total_pixels_avaliable_height = -1});

    int pixels = (response.height.pixels) ? (int)response.height.pixels : rect->h;
    pixels = max(pixels, rect->h);

    data->texture_width = rect->w;
    data->texture_height = pixels;
    SDL_DestroyTexture(data->texture);
    data->texture = NULL;
    SDLNW_SetWidgetNetSize(data->child, &(SDL_Rect){.w = data->texture_width, .h = data->texture_height});

    data->window = (SDL_Rect) {
        .x = 0,
        .y = 0,
        .w = rect->w,
        .h = rect->h
    };

    // setup vertical scroll
    data->y_window_max = max(data->texture_height - data->window.h, 0);
    data->y_scroll_bar = (SDL_Rect){0};
    if (data->texture_height > rect->h) {
        float y_proportion = (float)rect->h / (float)data->texture_height;

        data->y_scroll_bar.h = (int)(y_proportion * (float)rect->h);
        data->y_scroll_bar.w = 10;
        data->y_scroll_bar.x = rect->x + rect->w - data->y_scroll_bar.w;
    }

    // TODO horizontal scroll
}

static void scroll_destroy(SDLNW_Widget* w) {
    struct scroll_data* data = w->data;

    SDLNW_DestroyWidget(data->child);
    data->child = NULL;
    SDL_DestroyTexture(data->texture);
    data->texture = NULL;

    __sdlnw_free(w->data);
    w->data = NULL;
}

static SDL_SystemCursor scroll_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct scroll_data* data = w->data;

    return SDLNW_GetAppropriateCursorForWidget(data->child, x + data->window.x - w->net_size.x, y + data->window.y - w->net_size.y);
}

// need to offset x and y values of incoming events to account
// for x and y shift.
static void scroll_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct scroll_data* data = widget->data;

    SDLNW_MouseMotionEvent motion_event = {0};
    SDLNW_ClickEvent click_event = {0};
    SDLNW_DragEvent drag = {0};

    int x_offset = data->window.x - widget->net_size.x;
    int y_offset = data->window.y - widget->net_size.y;

    // todo others.
    if (type == SDLNW_EventType_Click) {
        // get offset from the scroll widget, then add offset of scroll
        SDLNW_ClickEvent* incoming = event_meta;

        click_event = *incoming;

        click_event.x = incoming->x + x_offset;
        click_event.y = incoming->y + y_offset;

        event_meta = &click_event;
    }
    else if (type == SDLNW_EventType_MouseMove) {
        SDLNW_MouseMotionEvent* incoming = event_meta;

        motion_event = (SDLNW_MouseMotionEvent){
            .current_x = incoming->current_x + x_offset,
            .current_y = incoming->current_y + y_offset,
            .last_x = incoming->last_x + x_offset,
            .last_y = incoming->last_y + y_offset
        };

        event_meta = &motion_event;
    }
    else if (type == SDLNW_EventType_MouseDrag) {
        // TODO save position on drag start, scroll might
        // change while dragging.
        SDLNW_DragEvent* incoming = event_meta;
        drag = *incoming;

        drag.mouse_x += x_offset;
        drag.mouse_y += y_offset;
        drag.origin_x += x_offset;
        drag.origin_y += y_offset;

        event_meta = &drag;
    }
    // TODO scroll should produce a mouce motion event,
    // since we may have scrolled somthing under/out from under the mouse.

    SDLNW_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

// TODO x
static void scroll_mouse_scroll(SDLNW_Widget* widget, SDLNW_MouseWheelEvent* event, bool* allow_passthrough) {
    struct scroll_data* data = widget->data;

    int delta_y = -20 * event->delta_y;

    *allow_passthrough = 0;
    data->window.y += delta_y;
    if (data->window.y < 0) {
        data->window.y = 0;
        *allow_passthrough = 1;
    }

    if (data->window.y > data->y_window_max) {
        data->window.y = data->y_window_max;
        *allow_passthrough = 1;
    }

    float y_proportion_scrolled = (float)data->window.y / (float)data->y_window_max;

    int y_scroll_max = widget->content_size.h - data->y_scroll_bar.h;

    data->y_scroll_bar.y = (int)(y_proportion_scrolled * (float)y_scroll_max) + widget->content_size.y;
}

static void scroll_drag(SDLNW_Widget* widget, SDLNW_DragEvent* event, bool* allow_passthrough) {
    (void)allow_passthrough; // unused for now, revisit when have more complicated examples since I can't forsee how it should work.
    struct scroll_data* data = widget->data;

    if (!data->dragging_y_scroll) {
        if (is_point_within_rect(event->mouse_x, event->mouse_y, &data->y_scroll_bar) && is_point_within_rect(event->origin_x, event->origin_y, &data->y_scroll_bar)) {
            data->dragging_y_scroll = true;
            data->start_scroll_drag_y = data->y_scroll_bar.y;
        }
    }

    if (data->dragging_y_scroll) {
        data->y_scroll_bar.y = data->start_scroll_drag_y + event->mouse_y - event->origin_y;

        if (data->y_scroll_bar.y < widget->content_size.y) {
            data->y_scroll_bar.y = widget->content_size.y;
        }

        int scroll_max = widget->content_size.y + widget->content_size.h - data->y_scroll_bar.h;
        if (data->y_scroll_bar.y > scroll_max) {
            data->y_scroll_bar.y = scroll_max;
        }

        int y_scroll_max = widget->content_size.y + widget->content_size.h - data->y_scroll_bar.h;
        float scrolled_proportion = (float)data->y_scroll_bar.y / (float)y_scroll_max;

        float max_scroll = (float)(data->texture_height - data->window.h);
        data->window.y = (int)(scrolled_proportion * max_scroll);
    }

    if (!event->still_down) {
        data->dragging_y_scroll = false;
    }
}

static SDLNW_SizeResponse scroll_get_requested_size(SDLNW_Widget* w, SDLNW_SizeRequest request) {
    (void)w;
    (void)request;
    return (SDLNW_SizeResponse) {
        .height = {
            .shares = 1
        },
        .width = {
            .shares = 1
        }
    };
}

SDLNW_Widget* SDLNW_CreateScrollWidget(SDLNW_Widget* child) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw_content = scroll_draw_content;
    widget->vtable.set_content_size = scroll_set_content_size;
    widget->vtable.destroy = scroll_destroy;
    widget->vtable.appropriate_cursor = scroll_appropriate_cursor;
    widget->vtable.trickle_down_event = scroll_trickle_down_event;
    widget->vtable.mouse_scroll = scroll_mouse_scroll;
    widget->vtable.drag = scroll_drag;
    widget->vtable.get_requested_size = scroll_get_requested_size;

    struct scroll_data* data = __sdlnw_malloc(sizeof(struct scroll_data));
    *data = (struct scroll_data) {.child = child};
    widget->data = data;

    SDLNW_SetWidgetNetSize(data->child, &widget->content_size);

    return widget;
}
