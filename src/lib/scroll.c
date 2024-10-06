#include "SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_render.h>

struct scroll_data {
    SDLNW_Widget* child;
    SDL_Rect child_window;

    SDL_Texture* texture;
    int texture_width, texture_height;

    SDL_Rect window;
    int y_window_max;

    SDL_Rect y_scroll_bar;
    bool dragging_y_scroll;
    int start_scroll_drag_x, start_scroll_drag_y;
};

static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
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

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct scroll_data* data = w->data;

    w->size = *rect;

    SDLNW_SizeRequest req = SDLNW_Widget_GetRequestedSize(data->child, SDLNW_SizingDimension_Width, rect->w);
    
    int pixels = (req.pixels) ? (int)req.pixels : rect->h;
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

static void destroy(SDLNW_Widget* w) {
    struct scroll_data* data = w->data;

    SDLNW_Widget_Destroy(data->child);
    data->child = NULL;
    SDL_DestroyTexture(data->texture);
    data->texture = NULL;

    free(w->data);
    w->data = NULL;
}

// TODO adjust x y
static SDL_SystemCursor appropriate_cursor(SDLNW_Widget* w, int x, int y) {
    (void)w; // unused
    (void)x; // unused
    (void)y; // unused
    return SDL_SYSTEM_CURSOR_ARROW;
}

// TODO adjust x y
static void trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, bool* allow_passthrough) {
    struct scroll_data* data = widget->data;

    SDLNW_Event_Click click_event = {0};

    // todo others.
    if (type == SDLNW_EventType_Click) {
        // get offset from the scroll widget, then add offset of scroll
        SDLNW_Event_Click* incoming = event_meta;
        int x_widget_offset = incoming->x - widget->size.x;
        int y_widget_offset = incoming->y - widget->size.y;

        click_event.x = x_widget_offset + data->window.x;
        click_event.y = y_widget_offset + data->window.y;

        event_meta = &click_event;
    }

    SDLNW_Widget_TrickleDownEvent(data->child, type, event_meta, allow_passthrough);
}

// TODO x
static void mouse_scroll(SDLNW_Widget* widget, SDLNW_Event_MouseWheel* event, bool* allow_passthrough) {   
    struct scroll_data* data = widget->data;

    int delta_y = -20 * event->y;
    
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

static void drag(SDLNW_Widget* widget, SDLNW_Event_Drag* event, bool* allow_passthrough) {
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

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.destroy = destroy;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    widget->vtable.trickle_down_event = trickle_down_event;
    widget->vtable.mouse_scroll = mouse_scroll;
    widget->vtable.drag = drag;

    struct scroll_data* data = malloc(sizeof(struct scroll_data));
    *data = (struct scroll_data) {.child = child};
    widget->data = data;

    SDLNW_Widget_Size(data->child, &widget->size);

    return widget;
}