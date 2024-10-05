#include "SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL_render.h>

struct scroll_data {
    SDLNW_Widget* child;
    SDL_Rect child_window;

    SDL_Texture* texture;
    int texture_width, texture_height;

    SDL_Rect window;
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

    // TODO horizontal scroll

    // vertical scroll
    if (data->texture_height > rect->h) {
        printf("requires scroll bar!\n");
    }
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
static void trickle_down_event(SDLNW_Widget* widget, enum SDLNW_EventType type, void* event_meta, int* allow_passthrough) {
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
static void mouse_scroll(SDLNW_Widget* widget, SDLNW_Event_MouseWheel* event, int* allow_passthrough) {   
    struct scroll_data* data = widget->data;

    int delta_y = -10 * event->y;
    
    *allow_passthrough = 0;
    data->window.y += delta_y;
    if (data->window.y < 0) {
        data->window.y = 0;
        *allow_passthrough = 1;
    }

    int y_max = max(data->texture_height - data->window.h, 0);
    if (data->window.y > y_max) {
        data->window.y = y_max;
        *allow_passthrough = 1;
    }
}

SDLNW_Widget* SDLNW_CreateScrollWidget(SDLNW_Widget* child) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.destroy = destroy;
    widget->vtable.appropriate_cursor = appropriate_cursor;
    // TODO, but for now default fine
    // widget->vtable.get_requested_size = get_requested_size;
    widget->vtable.trickle_down_event = trickle_down_event;
    widget->vtable.mouse_scroll = mouse_scroll;

    struct scroll_data* data = malloc(sizeof(struct scroll_data));
    *data = (struct scroll_data) {.child = child};
    widget->data = data;

    SDLNW_Widget_Size(data->child, &widget->size);

    return widget;
}