#include "../include/SDLNW.h"
#include "../include/internal_helpers.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_render.h>

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

static void scroll_draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct scroll_data* data = w->data;

    if (data->texture == NULL) {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(renderer, &info);
        data->texture = SDL_CreateTexture(renderer, info.texture_formats[0], SDL_TEXTUREACCESS_TARGET, data->texture_width, data->texture_height);
    }

    SDL_SetRenderTarget(renderer, data->texture);
    SDLNW_Widget_Draw(data->child, renderer);

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, data->texture, &data->window, &w->size);

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

static void scroll_size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct scroll_data* data = w->data;

    w->size = *rect;

    SDLNW_SizeResponse response = SDLNW_Widget_GetRequestedSize(data->child, (SDLNW_SizeRequest) {.total_pixels_avaliable_width = rect->w, .total_pixels_avaliable_height = -1});
    
    int pixels = (response.height.pixels) ? (int)response.height.pixels : rect->h;
    pixels = max(pixels, rect->h);

    data->texture_width = rect->w;
    data->texture_height = pixels;
    SDL_DestroyTexture(data->texture);
    data->texture = NULL;
    SDLNW_Widget_Size(data->child, &(SDL_Rect){.w = data->texture_width, .h = data->texture_height});

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

    SDLNW_Widget_Destroy(data->child);
    data->child = NULL;
    SDL_DestroyTexture(data->texture);
    data->texture = NULL;

    free(w->data);
    w->data = NULL;
}

static SDL_SystemCursor scroll_appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    struct scroll_data* data = w->data;

    return SDLNW_Widget_GetAppropriateCursor(data->child, x + data->window.x - w->size.x, y + data->window.y - w->size.y);
}

// need to offset x and y values of incoming events to account 
// for x and y shift.
static void scroll_trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct scroll_data* data = widget->data;

    SDLNW_Event_Click click_event = {0};
    SDLNW_Event_MouseMove motion_event = {0};

    int x_offset = data->window.x - widget->size.x;
    int y_offset = data->window.y - widget->size.y;

    // todo others.
    if (type == SDLNW_EventType_Click) {
        // get offset from the scroll widget, then add offset of scroll
        SDLNW_Event_Click* incoming = event_meta;

        click_event.x = incoming->x + x_offset;
        click_event.y = incoming->y + y_offset;

        event_meta = &click_event;
    }
    else if (type == SDLNW_EventType_MouseMove) {
        SDLNW_Event_MouseMove* incoming = event_meta;

        motion_event = (SDLNW_Event_MouseMove){
            .current_x = incoming->current_x + x_offset,
            .current_y = incoming->current_y + y_offset,
            .last_x = incoming->last_x + x_offset,
            .last_y = incoming->last_y + y_offset
        };

        event_meta = &motion_event;

    }
    // TODO scroll should produce a mouce motion event,
    // since we may have scrolled somthing under/out from under the mouse.

    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

// TODO x
static void scroll_mouse_scroll(SDLNW_Widget* widget, SDLNW_Event_MouseWheel* event, bool* allow_passthrough) {   
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

    int y_scroll_max = widget->size.h - data->y_scroll_bar.h;

    data->y_scroll_bar.y = (int)(y_proportion_scrolled * (float)y_scroll_max) + widget->size.y;
}

static void scroll_drag(SDLNW_Widget* widget, SDLNW_Event_Drag* event, bool* allow_passthrough) {
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

        if (data->y_scroll_bar.y < widget->size.y) {
            data->y_scroll_bar.y = widget->size.y;
        }

        int scroll_max = widget->size.y + widget->size.h - data->y_scroll_bar.h;
        if (data->y_scroll_bar.y > scroll_max) {
            data->y_scroll_bar.y = scroll_max;
        }

        int y_scroll_max = widget->size.y + widget->size.h - data->y_scroll_bar.h;
        float scrolled_proportion = (float)data->y_scroll_bar.y / (float)y_scroll_max;

        float max_scroll = (float)(data->texture_height - data->window.h);
        data->window.y = (int)(scrolled_proportion * max_scroll);
    }

    if (!event->still_down) {
        data->dragging_y_scroll = false;
    }
}

SDLNW_Widget* SDLNW_CreateScrollWidget(SDLNW_Widget* child) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = scroll_draw;
    widget->vtable.size = scroll_size;
    widget->vtable.destroy = scroll_destroy;
    widget->vtable.appropriate_cursor = scroll_appropriate_cursor;
    widget->vtable.trickle_down_event = scroll_trickle_down_event;
    widget->vtable.mouse_scroll = scroll_mouse_scroll;
    widget->vtable.drag = scroll_drag;

    struct scroll_data* data = malloc(sizeof(struct scroll_data));
    *data = (struct scroll_data) {.child = child};
    widget->data = data;

    SDLNW_Widget_Size(data->child, &widget->size);

    return widget;
}