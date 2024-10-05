#include "SDLNW.h"
#include "internal_helpers.h"
#include <SDL2/SDL_render.h>
#include <assert.h>
#include <stdbool.h>

struct text_pair {
    const char* text;
    SDL_Rect dims;
    SDL_Surface* surface;
    SDL_Texture* texture;

    struct text_pair* next;
};

// something that represents a break in a word
static int is_break(char c) {
    return c == ' ' || c == '.' || c == ',' || c == '?';
}

// seperate text into words, since we dont want to split words between lines
static struct text_pair* seperate_and_render_text(const char* original_text, SDLNW_Font* font) {
    struct text_pair* root = NULL;
    struct text_pair* last = NULL;

    // TODO
    SDL_Color fg = {0x00, 0x00, 0x00, 0xFF};

    uint cursor = 0;
    const uint len = strlen(original_text);

    int tries = 0;

    while (cursor < len && tries < 100000) {
        tries += 1;

        uint buffer_len = 0;
        while (len > cursor + buffer_len && !is_break(original_text[cursor + buffer_len])) {
            buffer_len += 1;
        }
        if (len > cursor + buffer_len) {
            // add an extra to include the break character
            buffer_len += 1;
        }

        // copy over
        char* buffer = malloc(sizeof(char) * (buffer_len + 1));
        for (uint i = 0; i < buffer_len; i++) {
            buffer[i] = original_text[cursor + i];
        }
        buffer[buffer_len] = '\0';

        struct text_pair* pair = malloc(sizeof(struct text_pair));
        *pair = (struct text_pair){ .text = buffer, .surface = TTF_RenderText_Solid(font->font, buffer, fg) };
        cursor += buffer_len;

        if (root == NULL) {
            root = pair;
            last = pair;
        } else {
            last->next = pair;
            last = pair;
        }
    }

    return root;
}

static void size_text(const struct text_pair* pair, int* cursor_x, int* cursor_y, const SDL_Rect* bounding_rect, SDL_Rect* text_destination) {
    text_destination->w = pair->surface->w;
    text_destination->h = pair->surface->h;

    if (pair->dims.w + *cursor_x > bounding_rect->w && *cursor_x != bounding_rect->x) {
        // newline
        *cursor_x = bounding_rect->x;
        *cursor_y += text_destination->h;
    }

    text_destination->x = *cursor_x;
    text_destination->y = *cursor_y;

    *cursor_x += pair->dims.w;
}

static void size_text_within(struct text_pair* pairs, const SDL_Rect* rect) {
    struct text_pair* pair = pairs;
    int cursor_x = rect->x;
    int cursor_y = rect->y;

    while(pair != NULL) {
        size_text(pair, &cursor_x, &cursor_y, rect, &pair->dims);
        pair = pair->next;
    }
}

static void render_text_pairs(struct text_pair* pairs, SDL_Renderer* renderer) {
    struct text_pair* p = pairs;

    while(p != NULL) {
        if (p->texture == NULL && p->surface != NULL) {
            p->texture = SDL_CreateTextureFromSurface(renderer, p->surface);
            assert(p->texture != NULL);
        }

        SDL_RenderCopy(renderer, p->texture, NULL, &p->dims);
        p = p->next;
    }
}

static void destroy_text_pairs(struct text_pair* pairs) {
    if (pairs == NULL) {
        return;
    }

    if (pairs->next) {
        destroy_text_pairs(pairs->next);
        pairs->next = NULL;
    }

    free((void*)pairs->text);
    free(pairs);
}

struct paragraph_data {
    SDLNW_Font* font;

    struct text_pair* rendered_text;
};



static void draw(SDLNW_Widget* w, SDL_Renderer* renderer) {
    struct paragraph_data* data = w->data;
    render_text_pairs(data->rendered_text, renderer);
}

static void size(SDLNW_Widget* w, const SDL_Rect* rect) {
    struct paragraph_data* data = w->data;

    w->size = *rect;
    size_text_within(data->rendered_text, rect);
}

static void destroy(SDLNW_Widget* w) {
    struct paragraph_data* data = w->data;

    destroy_text_pairs(data->rendered_text);
    data->rendered_text = NULL;
}

static SDLNW_SizeRequest get_requested_size(SDLNW_Widget* w, enum SDLNW_SizingDimension locked_dimension, uint dimension_pixels) {
    // TODO cache this?

    if (locked_dimension == SDLNW_SizingDimension_Height) {
        // meant to tell how wide based on height, TODO
        return (SDLNW_SizeRequest){0};
    }

    struct paragraph_data* data = w->data;
    SDLNW_SizeRequest req = (SDLNW_SizeRequest){0};
    SDL_Rect bounding_rect = (SDL_Rect) {.w = dimension_pixels};
    struct text_pair* pair = data->rendered_text;
    int cursor_x = 0;
    int cursor_y = 0;
    SDL_Rect text_dest;

    while(pair != NULL) {
        size_text(pair, &cursor_x, &cursor_y, &bounding_rect, &text_dest);
        pair = pair->next;
    }

    // height is going to be text_dest.y + text_dest.h
    req.pixels = text_dest.y + text_dest.h;

    return req;
}

SDLNW_Widget* SDLNW_CreateParagraphWidget(const char* text, SDLNW_Font* font) {
    SDLNW_Widget* widget = create_default_widget();

    widget->vtable.draw = draw;
    widget->vtable.size = size;
    widget->vtable.destroy = destroy;
    widget->vtable.get_requested_size = get_requested_size;

    widget->data = malloc(sizeof(struct paragraph_data));

    struct paragraph_data* data = widget->data;
    *data = (struct paragraph_data){0};

    data->rendered_text = seperate_and_render_text(text, font);
    data->font = font;

    return widget;
}
