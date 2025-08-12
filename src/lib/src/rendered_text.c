#include "internal_helpers.h"
#include <assert.h>

static void  destroy_rendered_char(__sdlnw_RenderedChar* c) {
    if (c->surface) {
        SDL_FreeSurface(c->surface);
    }

    if (c->texture) {
        SDL_DestroyTexture(c->texture);
    }
}

static void clear_chars(__sdlnw_RenderedText* rt) {
    for (size_t i = 0; i < rt->chars_len; i++) {
        destroy_rendered_char(&rt->chars[i]);
    }

    rt->chars_len = 0;
}

void __sdlnw_RenderedText_init(__sdlnw_RenderedText* rt, SDLNW_Font* font) {
    *rt = (__sdlnw_RenderedText) {
        .font = font
    };

    rt->chars_cap = 10;
    rt->chars = __sdlnw_malloc(rt->chars_cap * sizeof(__sdlnw_RenderedChar));
}

void __sdlnw_RenderedText_destroy(__sdlnw_RenderedText* rt) {
    __sdlnw_free(rt->chars);
    *rt = (__sdlnw_RenderedText){0};
}

// something that represents a break in a word
static bool is_break(char c) {
    return c == ' ' || c == '.' || c == ',' || c == '?';
}

void __sdlnw_RenderedText_set_text(__sdlnw_RenderedText* rt, const char* text, const SDL_Rect* to) {
    // clear existing
    clear_chars(rt);

    size_t len = strlen(text);

    // ensure enough space is allocated in buffer
    if (len >= rt->chars_cap) {
        while (len >= rt->chars_cap) {
            rt->chars_cap *= 2;
        }

        rt->chars = __sdlnw_realloc(rt->chars, rt->chars_cap * sizeof(__sdlnw_RenderedChar));
        assert(rt->chars != NULL);

        rt->position = *to;
    }

    char str[2] = {
        '\0',
        '\0'
    };

    // TODO
    SDL_Color fg = {0x00, 0x00, 0x00, 0xFF};
    SDL_Rect dest = {
        .x = to->x,
        .y = to->y
    };

    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        str[0] = c;

        if (c == '\n') {
            dest.x = to->x;
            dest.y += rt->font->line_height;
            dest.w = 0;
            dest.h = 0;

            rt->chars[i] = (__sdlnw_RenderedChar) {
                .c = c,
                .rendered_to = dest
            };

            continue;
        }

        SDL_Surface* surface = TTF_RenderText_Solid(rt->font->font, str, fg);

        dest.w = surface->w;
        dest.h = surface->h;

        rt->chars[i] = (__sdlnw_RenderedChar) {
            .c = c,
            .surface = surface,
            .native_width = surface->w,
            .native_height = surface->h,
            .rendered_to = dest
        };

        dest.x += dest.w;
        
        if (dest.x + dest.w > to->x + to->w) {
            // over ran width, backtrack until start of word
            size_t j = i;
            while (j > 0 && !is_break(text[j - 1])) {
                j--;
            }

            if (rt->chars[j].rendered_to.x == to->x) {
                // was line break anyway
                continue;
            }

            dest.x = to->x;
            dest.y += rt->font->line_height;
            for (; j <= i; j++) {
                dest.w = rt->chars[j].native_width;
                dest.h = rt->chars[j].native_height;
                rt->chars[j].rendered_to = dest;
                dest.x += dest.w;
            }
        }
    }

    rt->chars_len = len;
}

void __sdlnw_RenderedText_render_text(__sdlnw_RenderedText* rt, SDL_Renderer* renderer) {

    for (size_t i = 0; i < rt->chars_len; i++) {
        __sdlnw_RenderedChar * c = &rt->chars[i];

        if (c->surface) {
            if (!c->texture) {
                c->texture = SDL_CreateTextureFromSurface(renderer, c->surface);
            }

            SDL_RenderCopy(renderer, c->texture, NULL, &c->rendered_to);
        }
    }
}

void __sdlnw_RenderedText_render_cursor(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t cursor, const SDLNW_Colour* colour) {
    // find x and y of cursor
    int x, y, h;

    x = rt->position.x;
    y = rt->position.y;
    h = rt->font->line_height;

    if (cursor > 0) {
        __sdlnw_RenderedChar* c = &rt->chars[cursor - 1];
        x = c->rendered_to.x + c->rendered_to.w;
        y = c->rendered_to.y;
    }

    SDL_SetRenderDrawColor(renderer, colour->r, colour->g, colour->b, colour->a);
    SDL_RenderDrawLine(renderer, x, y, x, y + h);
}

void __sdlnw_RenderedText_render_highlight(__sdlnw_RenderedText* rt, SDL_Renderer* renderer, size_t from, size_t to, const SDLNW_Colour* colour) {
    SDL_SetRenderDrawColor(renderer, colour->r, colour->g, colour->b, colour->a);

    size_t low = from;
    size_t high = to;

    if (low > high) {
        high = from;
        low = to;
    }

    for (size_t i = low; i < high; i++) {
        SDL_RenderFillRect(renderer, &rt->chars[i].rendered_to);
    }
}

size_t __sdlnw_RenderedText_convert_point_to_char_index(__sdlnw_RenderedText* rt, int x, int y) {
    // TODO y
    (void)y;
    size_t index = (size_t)-1;

    for (size_t i = 0; i < rt->chars_len; i++) {
        __sdlnw_RenderedChar* c = &rt->chars[i];

        if (x < (c->rendered_to.x + c->rendered_to.w/2) &&
            y >= c->rendered_to.y &&
            y < c->rendered_to.y + c->rendered_to.h) {
                
            index = i;
            break;
        }
    }

    return index;
}
